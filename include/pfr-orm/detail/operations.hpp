#pragma once

#include <pfr-orm/definitions.hpp>
#include <pfr-orm/postgres/utils.hpp>

namespace pfrorm::postgres::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

} // namespace pfrorm::postgres::detail
