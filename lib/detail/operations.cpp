#include "pfr-orm/definitions.hpp"
#include "pfr-orm/postges-helpers.hpp"

#include <iterator>

#include <fmt/core.h>
#include <fmt/format.h>
#include <libpq-fe.h>

namespace pfrorm::postgres::detail {

void createTable(PGconn &connection, const EntityDescription &entity) {
  const Str escapedTableName = escapeIdentifier(connection, entity.name);
  execute(connection, fmt::format("DROP TABLE IF EXISTS {}", escapedTableName));

  fmt::memory_buffer buf;
  auto inserter = std::back_inserter(buf);
  fmt::format_to(inserter, "CREATE TABLE {} (", escapedTableName);
  for (bool first = true; const FieldDescription &field : entity.fields) {
    const Str escapedFieldName = escapeIdentifier(connection, field.name);

    fmt::format_to(inserter, "{}{} {}", first ? "" : ", ", escapedFieldName,
                   field.nativeType);

    first = false;
  }
  fmt::format_to(inserter, ")");
  execute(connection, fmt::to_string(buf));
}

bool exists(PGconn &connection, const EntityDescription &entity) {
  const Result result =
      query(connection, fmt::format("SELECT EXISTS(SELECT 1 FROM {})",
                                    escapeIdentifier(connection, entity.name)));
  return *PQgetvalue(result.get(), 0, 0) == 't';
}

} // namespace pfrorm::postgres::detail
