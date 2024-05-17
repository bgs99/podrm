#include "../detail/multilambda.hpp"

#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>
#include <pfr-orm/sqlite/detail/operations.hpp>
#include <pfr-orm/sqlite/utils.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>

namespace pfrorm::sqlite::detail {

namespace {

std::string_view toString(const NativeType type) {
  switch (type) {
  case NativeType::BigInt:
    return "INTEGER";
  case NativeType::String:
    return "TEXT";
  }
  throw std::runtime_error{
      "Unsupported native type " + std::to_string(static_cast<int>(type)),
  };
}

void createTableFields(const FieldDescription &description,
                       const bool isPrimaryKey, fmt::appender &appender,
                       std::vector<std::string_view> prefixes, bool &first) {
  prefixes.push_back(description.name);

  const auto createPrimitiveField =
      [isPrimaryKey, &prefixes, &appender,
       &first](const PrimitiveFieldDescription &descr) {
        fmt::format_to(appender, "{}'{}' '{}'{}", first ? "" : ",",
                       fmt::to_string(fmt::join(prefixes, "_")),
                       toString(descr.nativeType),
                       isPrimaryKey ? " PRIMARY KEY" : "");
        first = false;
      };

  const auto createCompositeField =
      [&prefixes, &appender, &first](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createTableFields(field, false, appender, prefixes, first);
        }
      };

  const pfrorm::detail::MultiLambda createField{
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
  connection.execute(fmt::format("DROP TABLE IF EXISTS '{}'", entity.name));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE '{}' (", entity.name);
  bool first = true;
  for (std::size_t i = 0; i < entity.fields.size(); ++i) {
    createTableFields(entity.fields[i], entity.primaryKey == i, appender, {},
                      first);
  }
  fmt::format_to(appender, ")");
  connection.execute(fmt::to_string(buf));
}

bool exists(Connection &connection, const EntityDescription &entity) {
  const Result result = connection.query(
      fmt::format("SELECT EXISTS(SELECT 1 FROM '{}')", entity.name));
  return result.getRow()->boolean(0);
}

} // namespace pfrorm::sqlite::detail
