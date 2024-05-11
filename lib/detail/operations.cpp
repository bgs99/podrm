#include "multilambda.hpp"
#include "pfr-orm/api.hpp"
#include "pfr-orm/definitions.hpp"
#include "pfr-orm/postges-helpers.hpp"
#include <cstddef>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <libpq-fe.h>

namespace pfrorm::postgres::detail {

namespace {

std::string_view toString(const NativeType type) {
  switch (type) {
  case NativeType::BigInt:
    return "BIGINT";
  case NativeType::String:
    return "VARCHAR";
  }
}

// NOLINTNEXTLINE(misc-no-recursion): by design
void createTableFields(const FieldDescription &description,
                       const bool isPrimaryKey, PGconn &connection,
                       fmt::appender &appender,
                       std::vector<std::string_view> prefixes, bool &first) {
  prefixes.push_back(description.name);

  const auto createPrimitiveField =
      [isPrimaryKey, &prefixes, &connection, &appender,
       &first](const PrimitiveFieldDescription &descr) {
        fmt::format_to(
            appender, "{}{} {}{}", first ? "" : ",",
            escapeIdentifier(connection,
                             fmt::to_string(fmt::join(prefixes, "_"))),
            toString(descr.nativeType), isPrimaryKey ? " PRIMARY KEY" : "");
        first = false;
      };

  const auto createCompositeField =
      // NOLINTNEXTLINE(misc-no-recursion): by design
      [&prefixes, &connection, &appender,
       &first](const CompositeFieldDescription &descr) {
        for (const FieldDescription &field : descr.fields) {
          createTableFields(field, false, connection, appender, prefixes,
                            first);
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

void createTable(PGconn &connection, const EntityDescription &entity) {
  const Str escapedTableName = escapeIdentifier(connection, entity.name);
  execute(connection, fmt::format("DROP TABLE IF EXISTS {}", escapedTableName));

  fmt::memory_buffer buf;
  fmt::appender appender{buf};
  fmt::format_to(appender, "CREATE TABLE {} (", escapedTableName);
  bool first = true;
  for (std::size_t i = 0; i < entity.fields.size(); ++i) {
    createTableFields(entity.fields[i], entity.primaryKey == i, connection,
                      appender, {}, first);
  }
  fmt::format_to(appender, ")");
  execute(connection, fmt::to_string(buf));
}

bool exists(PGconn &connection, const EntityDescription &entity) {
  const Result result =
      query(connection, fmt::format("SELECT EXISTS(SELECT 1 FROM {})",
                                    escapeIdentifier(connection, entity.name)));
  return *PQgetvalue(result.get(), 0, 0) == 't';
}

} // namespace pfrorm::postgres::detail
