#include "../detail/multilambda.hpp"

#include <podrm/api.hpp>
#include <podrm/reflection.hpp>
#include <podrm/span.hpp>
#include <podrm/sqlite/detail/connection.hpp>
#include <podrm/sqlite/detail/result.hpp>
#include <podrm/sqlite/detail/row.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <sqlite3.h>

namespace podrm::sqlite::detail {

namespace {

using Statement =
    std::unique_ptr<sqlite3_stmt, decltype([](sqlite3_stmt *statement) {
                      sqlite3_finalize(statement);
                    })>;

Statement createStatement(sqlite3 &connection,
                          const std::string_view statement) {
  sqlite3_stmt *stmt = nullptr;
  const int result =
      sqlite3_prepare_v3(&connection, statement.data(),
                         static_cast<int>(statement.size()), 0, &stmt, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errmsg(&connection)};
  }

  return Statement{stmt};
}

void bindArg(const Statement &statement, const int pos, const AsImage &value) {
  const auto bindInt = [&statement, pos](const std::int64_t value) {
    sqlite3_bind_int64(statement.get(), pos + 1, value);
  };
  const auto bindUInt = [&statement, pos](const std::uint64_t value) {
    if (value > std::numeric_limits<std::int64_t>::max()) {
      throw std::invalid_argument{"Unsigned integer too big"};
    }
    sqlite3_bind_int64(statement.get(), pos + 1,
                       static_cast<std::int64_t>(value));
  };
  const auto bindBlob = [&statement, pos](const span<const std::byte> blob) {
    sqlite3_bind_blob64(statement.get(), pos, blob.data(), blob.size(),
                        SQLITE_STATIC);
  };
  const auto bindDouble = [&statement, pos](const double value) {
    sqlite3_bind_double(statement.get(), pos + 1, value);
  };
  const auto bindText = [&statement, pos](const std::string_view text) {
    sqlite3_bind_text64(statement.get(), pos + 1, text.data(), text.size(),
                        SQLITE_STATIC, SQLITE_UTF8);
  };
  const auto bindBool = [&statement, pos](const bool value) {
    sqlite3_bind_int(statement.get(), pos + 1, value ? 1 : 0);
  };

  std::visit(podrm::detail::MultiLambda{bindBlob, bindDouble, bindText, bindInt,
                                        bindUInt, bindBool},
             value);
}

std::string_view toString(const ImageType type) {
  switch (type) {
  case ImageType::Bool:
  case ImageType::Int:
  case ImageType::Uint:
    return "INTEGER";
  case ImageType::String:
    return "TEXT";
  case ImageType::Float:
    return "REAL";
  case ImageType::Bytes:
    return "BLOB";
  }
  throw std::runtime_error{
      "Unsupported image type " + std::to_string(static_cast<int>(type)),
  };
}

void createFields(const FieldDescription &description, const bool isPrimaryKey,
                  fmt::appender &appender,
                  std::vector<std::string_view> prefixes, bool &first) {
  prefixes.push_back(description.name);

  const auto createPrimitiveField =
      [isPrimaryKey, &prefixes, &appender,
       &first](const PrimitiveFieldDescription &descr) {
        fmt::format_to(appender, "{}'{}' '{}'{}", first ? "" : ",",
                       fmt::to_string(fmt::join(prefixes, "_")),
                       toString(descr.imageType),
                       isPrimaryKey ? " PRIMARY KEY" : "");
        first = false;
      };

  const auto createCompositeField =
      [&prefixes, &appender, &first](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createFields(field, false, appender, prefixes, first);
        }
      };

  std::visit(
      podrm::detail::MultiLambda{
          createPrimitiveField,
          createCompositeField,
      },
      description.field);
}

void createConstraints(const FieldDescription &description,
                       fmt::appender &appender,
                       std::vector<std::string_view> prefixes) {
  prefixes.push_back(description.name);

  const auto createPrimitiveFieldConstraints =
      [&prefixes, &appender](const PrimitiveFieldDescription &descr) {
        if (!descr.foreignKeyContraint.has_value()) {
          return;
        }

        fmt::format_to(appender, ", FOREIGN KEY('{}') REFERENCES '{}'('{}')",
                       fmt::to_string(fmt::join(prefixes, "_")),
                       descr.foreignKeyContraint->entity,
                       descr.foreignKeyContraint->field);
      };

  const auto createCompositeFieldConstraints =
      [&prefixes, &appender](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createConstraints(field, appender, prefixes);
        }
      };

  std::visit(
      podrm::detail::MultiLambda{
          createPrimitiveFieldConstraints,
          createCompositeFieldConstraints,
      },
      description.field);
}

void createPersistParamPlaceholders(const FieldDescription description,
                                    fmt::appender &appender, bool &first) {
  const auto createPrimitive = [&appender,
                                &first](const PrimitiveFieldDescription) {
    fmt::format_to(appender, "{}?", first ? "" : ",");
    first = false;
  };

  const auto createComposite = [&appender,
                                &first](const CompositeFieldDescription descr) {
    for (const FieldDescription field : descr.fields) {
      createPersistParamPlaceholders(field, appender, first);
    }
  };

  std::visit(podrm::detail::MultiLambda{createPrimitive, createComposite},
             description.field);
}

void createUpdateParamPlaceholders(const FieldDescription &description,
                                   fmt::appender &appender,
                                   std::vector<std::string_view> prefixes,
                                   bool &first) {
  prefixes.push_back(description.name);

  const auto createPrimitiveField = [&prefixes, &appender,
                                     &first](const PrimitiveFieldDescription) {
    fmt::format_to(appender, "{}'{}'=?", first ? "" : ",",
                   fmt::to_string(fmt::join(prefixes, "_")));
    first = false;
  };

  const auto createCompositeField =
      [&prefixes, &appender, &first](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createUpdateParamPlaceholders(field, appender, prefixes, first);
        }
      };

  std::visit(
      podrm::detail::MultiLambda{
          createPrimitiveField,
          createCompositeField,
      },
      description.field);
}

std::vector<AsImage> intoArgs(const FieldDescription description,
                              const void *field) {
  const auto createPrimitive =
      [field](
          const PrimitiveFieldDescription description) -> std::vector<AsImage> {
    return {description.asImage(field)};
  };

  const auto createComposite =
      [field](const CompositeFieldDescription descr) -> std::vector<AsImage> {
    std::vector<AsImage> values;
    for (const FieldDescription fieldDescr : descr.fields) {
      for (AsImage &value :
           intoArgs(fieldDescr, fieldDescr.constMemberPtr(field))) {
        values.emplace_back(std::move(value));
      }
    }

    return values;
  };

  return std::visit(
      podrm::detail::MultiLambda{createPrimitive, createComposite},
      description.field);
}

void init(const FieldDescription description, const Row row, int &currentColumn,
          void *field) {
  const auto initPrimitive = [field, row, &currentColumn](
                                 const PrimitiveFieldDescription description) {
    switch (description.imageType) {
    case ImageType::Int:
      description.fromImage(row.get(currentColumn).bigint(), field);
      ++currentColumn;
      return;
    case ImageType::Uint:
      description.fromImage(
          static_cast<std::uint64_t>(row.get(currentColumn).bigint()), field);
      ++currentColumn;
      return;
    case ImageType::String:
      description.fromImage(row.get(currentColumn).text(), field);
      ++currentColumn;
      return;
    case ImageType::Float:
      description.fromImage(row.get(currentColumn).real(), field);
      ++currentColumn;
      return;
    case ImageType::Bool:
      description.fromImage(row.get(currentColumn).boolean(), field);
      ++currentColumn;
      return;
    case ImageType::Bytes:
      description.fromImage(row.get(currentColumn).bytes(), field);
      ++currentColumn;
      return;
    }
    assert(false);
  };

  const auto initComposite = [field, row, &currentColumn](
                                 const CompositeFieldDescription description) {
    for (const FieldDescription fieldDescr : description.fields) {
      init(fieldDescr, row, currentColumn, fieldDescr.memberPtr(field));
    }
  };

  std::visit(podrm::detail::MultiLambda{initPrimitive, initComposite},
             description.field);
}

} // namespace

Connection::Connection(sqlite3 &connection)
    : connection(&connection, &sqlite3_close_v2) {
  this->execute("PRAGMA foreign_keys = ON");
}

Connection Connection::fromRaw(sqlite3 &connection) {
  return Connection{connection};
}

Connection Connection::inMemory(const char *const name) {
  sqlite3 *connection = nullptr;
  const int result = sqlite3_open_v2(
      name, &connection,
      SQLITE_OPEN_MEMORY | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errstr(result)};
  }

  return Connection{*connection};
}

Connection Connection::inFile(const std::filesystem::path &path) {
  sqlite3 *connection = nullptr;
  const int result =
      sqlite3_open_v2(path.string().c_str(), &connection,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errstr(result)};
  }

  return Connection{*connection};
}

std::uint64_t Connection::execute(const std::string_view statement,
                                  const span<const AsImage> args) {
  const std::unique_lock lock{*this->mutex};

  const Statement stmt = createStatement(*this->connection, statement);

  for (int i = 0; i < args.size(); ++i) {
    bindArg(stmt, i, args[i]);
  }

  const int executeResult = sqlite3_step(stmt.get());
  if (executeResult != SQLITE_DONE) {
    throw std::runtime_error{sqlite3_errmsg(this->connection.get())};
  }

  return sqlite3_changes64(this->connection.get());
}

Result Connection::query(const std::string_view statement,
                         const span<const AsImage> args) {
  const std::unique_lock lock{*this->mutex};

  Statement stmt = createStatement(*this->connection, statement);

  for (int i = 0; i < args.size(); ++i) {
    bindArg(stmt, i, args[i]);
  }

  return Result{{stmt.release(), &sqlite3_finalize}};
}

void Connection::createTable(const EntityDescription &entity) {
  this->execute(fmt::format("DROP TABLE IF EXISTS '{}'", entity.name));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE '{}' (", entity.name);

  bool first = true;
  for (std::size_t i = 0; i < entity.fields.size(); ++i) {
    createFields(entity.fields[i], entity.primaryKey == i, appender, {}, first);
  }

  for (const FieldDescription &field : entity.fields) {
    createConstraints(field, appender, {});
  }

  fmt::format_to(appender, ")");
  this->execute(fmt::to_string(buf));
}

bool Connection::exists(const EntityDescription &entity) {
  const Result result = this->query(
      fmt::format("SELECT EXISTS(SELECT 1 FROM '{}')", entity.name));
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access): fixed query
  return result.getRow().value().get(0).boolean();
}

void Connection::persist(const EntityDescription &description, void *entity) {
  fmt::memory_buffer buf;
  fmt::appender appender{buf};

  fmt::format_to(appender, "INSERT INTO '{}' VALUES (", description.name);

  bool first = true;
  std::vector<AsImage> values;
  for (std::size_t i = 0; i < description.fields.size(); ++i) {
    if (description.idMode == IdMode::Auto && i == description.primaryKey) {
      // TODO: support auto ids
    }
    createPersistParamPlaceholders(description.fields[i], appender, first);

    for (AsImage &value :
         intoArgs(description.fields[i],
                  description.fields[i].constMemberPtr(entity))) {
      values.emplace_back(std::move(value));
    }
  }
  fmt::format_to(appender, ")");

  this->execute(fmt::to_string(buf), values);
}

bool Connection::find(const EntityDescription &description, const AsImage &key,
                      void *result) {
  const std::string queryStr =
      fmt::format("SELECT * FROM '{}' WHERE {} = ?", description.name,
                  description.fields[description.primaryKey].name);

  const Result query =
      this->query(queryStr, podrm::span<const AsImage, 1>{&key, 1});
  std::optional<Row> row = query.getRow();
  if (!row.has_value()) {
    return false;
  }

  int currentColumn = 0;
  for (const FieldDescription description : description.fields) {
    init(description, row.value(), currentColumn,
         description.memberPtr(result));
  }

  return true;
}

void Connection::erase(const EntityDescription description,
                       const AsImage &key) {
  const std::string queryStr =
      fmt::format("DELETE FROM '{}' WHERE {} = ?", description.name,
                  description.fields[description.primaryKey].name);

  const std::uint64_t changes =
      this->execute(queryStr, podrm::span<const AsImage, 1>{&key, 1});
  if (changes == 0) {
    throw std::runtime_error("Entity with the given key is not found");
  }
}

void Connection::update(const EntityDescription description,
                        const void *entity) {
  fmt::memory_buffer buf;
  fmt::appender appender{buf};

  fmt::format_to(appender, "UPDATE '{}' SET ", description.name);

  bool first = true;
  std::vector<AsImage> values;
  for (const FieldDescription field : description.fields) {
    createUpdateParamPlaceholders(field, appender, {}, first);
    for (AsImage &value : intoArgs(field, field.constMemberPtr(entity))) {
      values.emplace_back(std::move(value));
    }
  }

  fmt::format_to(appender, " WHERE {} = ?",
                 description.fields[description.primaryKey].name);

  std::vector<AsImage> key = intoArgs(
      description.fields[description.primaryKey],
      description.fields[description.primaryKey].constMemberPtr(entity));
  if (key.size() != 1) {
    throw std::invalid_argument{
        fmt::format("Entity has composite primary key")};
  }
  values.emplace_back(std::move(key[0]));

  const std::uint64_t changes = this->execute(fmt::to_string(buf), values);
  if (changes == 0) {
    throw std::runtime_error("Entity with the given key is not found");
  }
}

} // namespace podrm::sqlite::detail
