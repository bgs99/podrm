#include "error.hpp"
#include "string.hpp"

#include <podrm/metadata.hpp>
#include <podrm/multilambda.hpp>
#include <podrm/odbc/detail/connection.hpp>
#include <podrm/odbc/detail/cursor.hpp>
#include <podrm/odbc/detail/result.hpp>
#include <podrm/odbc/detail/row.hpp>
#include <podrm/odbc/environment.hpp>
#include <podrm/span.hpp>

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
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sqlucode.h>

namespace podrm::odbc::detail {

namespace {

using Statement = std::unique_ptr<void, decltype([](SQLHSTMT statement) {
                                    SQLFreeStmt(statement, 0);
                                  })>;

Statement createStatement(SQLHDBC const connection,
                          const std::string_view statement) {
  SQLHSTMT stmt = nullptr;
  SQLAllocStmt(connection, &stmt);

  const String odbcStatement = detail::makeString(statement);
  const int result = SQLPrepare(stmt, odbcStatement.data(),
                                static_cast<SQLSMALLINT>(odbcStatement.size()));
  if (!SQL_SUCCEEDED(result)) {
    throw std::runtime_error{extractError(connection, SQL_HANDLE_DBC)};
  }

  return Statement{stmt};
}

void bindArg(const Statement &statement, const int pos, const AsImage &value) {
  const auto bindInt = [&statement, pos](const std::int64_t &value) {
    SQLBindParameter(
        statement.get(), pos + 1, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0,
        0,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): not changed
        const_cast<void *>(static_cast<const void *>(&value)), 0, nullptr);
  };
  const auto bindUInt = [&statement, pos](const std::uint64_t &value) {
    SQLBindParameter(
        statement.get(), pos + 1, SQL_PARAM_INPUT, SQL_C_UBIGINT, SQL_BIGINT, 0,
        0,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): not changed
        const_cast<void *>(static_cast<const void *>(&value)), 0, nullptr);
  };
  const auto bindBlob = [&statement, pos](const span<const std::byte> blob) {
    SQLBindParameter(
        statement.get(), pos + 1, SQL_PARAM_INPUT, SQL_C_BINARY,
        SQL_LONGVARBINARY, 0, 0,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): not changed
        const_cast<void *>(static_cast<const void *>(blob.data())),
        static_cast<SQLLEN>(blob.size()), nullptr);
  };
  const auto bindDouble = [&statement, pos](const double value) {
    SQLBindParameter(
        statement.get(), pos + 1, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0,
        0,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): not changed
        const_cast<void *>(static_cast<const void *>(&value)), 0, nullptr);
  };
  const auto bindText = [&statement, pos](const std::string_view text) {
    SQLBindParameter(
        statement.get(), pos + 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0,
        0,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): not changed
        const_cast<void *>(static_cast<const void *>(text.data())),
        static_cast<SQLLEN>(text.size()), nullptr);
  };
  const auto bindBool = [&statement, pos](const bool value) {
    SQLBindParameter(
        statement.get(), pos + 1, SQL_PARAM_INPUT, SQL_C_BIT, SQL_BIT, 0, 0,
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): not changed
        const_cast<void *>(static_cast<const void *>(&value)), 0, nullptr);
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
        fmt::format_to(appender, "{}\"{}\" {}{}", first ? "" : ",",
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

        fmt::format_to(appender, R"(, FOREIGN KEY("{}") REFERENCES "{}"("{}"))",
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
    fmt::format_to(appender, R"({} "{}"=?)", first ? "" : ",",
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

void closeConnection(SQLHDBC connection) { SQLDisconnect(connection); }

} // namespace

Connection::Connection(SQLHANDLE connection)
    : connection(connection, &closeConnection) {}

Connection Connection::fromRaw(SQLHANDLE connection) {
  return Connection{connection};
}

Connection
Connection::fromConnectionString(Environment &environment,
                                 const std::string_view connectionString) {
  SQLHDBC connection = nullptr;
  SQLAllocHandle(SQL_HANDLE_DBC, environment.getRaw(), &connection);

  const String odbcConnectionString = detail::makeString(connectionString);

  const int result =
      SQLDriverConnect(connection, nullptr, odbcConnectionString.data(),
                       static_cast<SQLSMALLINT>(connectionString.size()),
                       nullptr, 0, nullptr, SQL_DRIVER_COMPLETE);

  if (!SQL_SUCCEEDED(result)) {
    throw std::runtime_error{extractError(connection, SQL_HANDLE_DBC)};
  }

  return Connection{connection};
}

std::uint64_t Connection::execute(const std::string_view statement,
                                  const span<const AsImage> args) {
  const std::unique_lock lock{*this->mutex};

  const Statement stmt = createStatement(this->connection.get(), statement);

  for (int i = 0; i < args.size(); ++i) {
    bindArg(stmt, i, args[i]);
  }

  const int executeResult = SQLExecute(stmt.get());
  if (!SQL_SUCCEEDED(executeResult)) {
    throw std::runtime_error{
        extractError(this->connection.get(), SQL_HANDLE_DBC)};
  }

  SQLLEN affectedRows = 0;
  SQLRowCount(stmt.get(), &affectedRows);
  return affectedRows;
}

Result Connection::query(const std::string_view statement,
                         const span<const AsImage> args) {
  const std::unique_lock lock{*this->mutex};

  Statement stmt = createStatement(this->connection.get(), statement);

  for (int i = 0; i < args.size(); ++i) {
    bindArg(stmt, i, args[i]);
  }

  const int executeResult = SQLExecute(stmt.get());
  if (!SQL_SUCCEEDED(executeResult)) {
    throw std::runtime_error{
        extractError(this->connection.get(), SQL_HANDLE_DBC)};
  }

  return Result{std::move(stmt)};
}

void Connection::createTable(const EntityDescription &entity) {
  this->execute(fmt::format("DROP TABLE IF EXISTS \"{}\"", entity.name));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE \"{}\" (", entity.name);

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

void Connection::dropTable(const EntityDescription &entity) {
  this->execute(fmt::format(R"(DROP TABLE "{}")", entity.name));
}

bool Connection::exists(const EntityDescription &entity) {
  const Result result = this->query(
      fmt::format(R"(SELECT EXISTS(SELECT 1 FROM "{}"))", entity.name));
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access): fixed query
  return result.getRow().value().get(0).boolean();
}

void Connection::persist(const EntityDescription &description, void *entity) {
  fmt::memory_buffer buf;
  fmt::appender appender{buf};

  fmt::format_to(appender, R"(INSERT INTO "{}" VALUES ()", description.name);

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
      fmt::format(R"(SELECT * FROM "{}" WHERE {} = ?)", description.name,
                  description.fields[description.primaryKey].name);

  const Cursor cursor = Cursor{
      this->query(queryStr, podrm::span<const AsImage, 1>{&key, 1}),
      description.fields,
  };

  return cursor.extract(result);
}

void Connection::erase(const EntityDescription description,
                       const AsImage &key) {
  const std::string queryStr =
      fmt::format(R"(DELETE FROM "{}" WHERE {} = ?)", description.name,
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

  fmt::format_to(appender, R"(UPDATE "{}" SET )", description.name);

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

Cursor Connection::iterate(const EntityDescription description) {
  const std::string queryStr =
      fmt::format(R"(SELECT * FROM "{}")", description.name);

  return Cursor{
      this->query(queryStr),
      description.fields,
  };
}

} // namespace podrm::odbc::detail
