#include "../detail/multilambda.hpp"
#include "formatters.hpp" // IWYU pragma: keep

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>
#include <podrm/postgres/utils.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

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

void createTable(Connection &connection, const EntityDescription &entity) {
  const Str escapedTableName = connection.escapeIdentifier(entity.name);
  connection.execute(fmt::format("DROP TABLE IF EXISTS {}", escapedTableName));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE {} (", escapedTableName);
  bool first = true;
  for (std::size_t i = 0; i < entity.fields.size(); ++i) {
    createTableFields(entity.fields[i], entity.primaryKey == i, connection,
                      appender, {}, first);
  }
  fmt::format_to(appender, ")");
  connection.execute(fmt::to_string(buf));
}

bool exists(Connection &connection, const EntityDescription &entity) {
  const Result result =
      connection.query(fmt::format("SELECT EXISTS(SELECT 1 FROM {})",
                                   connection.escapeIdentifier(entity.name)));
  return result.value(0, 0)[0] == 't';
}

} // namespace podrm::postgres::detail
