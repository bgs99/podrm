#pragma once

#include <pfr-orm/definitions.hpp>

#include <libpq-fe.h>

namespace pfrorm::postgres::detail {

void createTable(PGconn &connection, const EntityDescription &entity);

bool exists(PGconn &connection, const EntityDescription &entity);

} // namespace pfrorm::postgres::detail
