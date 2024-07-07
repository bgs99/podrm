#include "formatters.hpp" // IWYU pragma: keep

#include <podrm/metadata.hpp>
#include <podrm/multilambda.hpp>
#include <podrm/postgres/detail/connection.hpp>
#include <podrm/postgres/detail/result.hpp>
#include <podrm/postgres/detail/str.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <libpq-fe.h>

namespace podrm::postgres::detail {

namespace {

std::string_view toString(const ImageType type) {
  switch (type) {
  case ImageType::Int:
  case ImageType::Uint:
    return "BIGINT";
  case ImageType::String:
    return "VARCHAR";
  case ImageType::Float:
    return "DOUBLE PRECISION";
  case ImageType::Bool:
    return "BOOLEAN";
  case ImageType::Bytes:
    return "BYTEA";
  }
  throw std::runtime_error{
      "Unsupported image type " + std::to_string(static_cast<int>(type)),
  };
}

void createTableFields(const FieldDescription &description,
                       const bool isPrimaryKey, Connection &connection,
                       fmt::appender &appender,
                       std::vector<std::string_view> prefixes, bool &first) {
  prefixes.push_back(description.name);

  const auto createPrimitiveField =
      [isPrimaryKey, &prefixes, &connection, &appender,
       &first](const PrimitiveFieldDescription &descr) {
        fmt::format_to(appender, "{}{} {}{}", first ? "" : ",",
                       connection.escapeIdentifier(
                           fmt::to_string(fmt::join(prefixes, "_"))),
                       toString(descr.imageType),
                       isPrimaryKey ? " PRIMARY KEY" : "");
        first = false;
      };

  const auto createCompositeField =
      [&prefixes, &connection, &appender,
       &first](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createTableFields(field, false, connection, appender, prefixes,
                            first);
        }
      };

  const podrm::detail::MultiLambda createField{
      createPrimitiveField,
      createCompositeField,
  };

  static_assert(std::is_invocable_r_v<void, decltype(createField),
                                      const PrimitiveFieldDescription &>);
  static_assert(std::is_invocable_r_v<void, decltype(createField),
                                      const CompositeFieldDescription &>);

  std::visit(createField, description.field);
}

} // namespace

Connection::Connection(const std::string &connectionStr)
    : connection(PQconnectdb(connectionStr.c_str())) {
  if (PQstatus(this->connection) != CONNECTION_OK) {
    throw std::runtime_error{fmt::format("Failed to connect to db: {}",
                                         PQerrorMessage(this->connection))};
  }
}

Connection::~Connection() { PQfinish(this->connection); }

Str Connection::escapeIdentifier(const std::string_view identifier) const {
  return Str{PQescapeIdentifier(this->connection, identifier.data(),
                                identifier.size())};
}

Result Connection::execute(const std::string &statement) {
  Result result{PQexec(this->connection, statement.c_str())};
  if (result.status() != PGRES_COMMAND_OK) {
    throw std::runtime_error{
        fmt::format("Error when executing a statement: {}",
                    PQerrorMessage(this->connection)),
    };
  }
  return result;
}

Result Connection::query(const std::string &statement) {
  Result result{PQexec(this->connection, statement.c_str())};
  if (result.status() != PGRES_TUPLES_OK) {
    throw std::runtime_error{
        fmt::format("Error when executing a query: {}",
                    PQerrorMessage(this->connection)),
    };
  }
  return result;
}

void Connection::createTable(const EntityDescription &entity) {
  const Str escapedTableName = this->escapeIdentifier(entity.name);
  this->execute(fmt::format("DROP TABLE IF EXISTS {}", escapedTableName));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE {} (", escapedTableName);
  bool first = true;
  for (std::size_t i = 0; i < entity.fields.size(); ++i) {
    createTableFields(entity.fields[i], entity.primaryKey == i, *this, appender,
                      {}, first);
  }
  fmt::format_to(appender, ")");
  this->execute(fmt::to_string(buf));
}

bool Connection::exists(const EntityDescription &entity) {
  const Result result = this->query(fmt::format(
      "SELECT EXISTS(SELECT 1 FROM {})", this->escapeIdentifier(entity.name)));
  return result.value(0, 0)[0] == 't';
}

} // namespace podrm::postgres::detail
