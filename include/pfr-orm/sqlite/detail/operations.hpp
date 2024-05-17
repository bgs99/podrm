#pragma once

#include <pfr-orm/definitions.hpp>
#include <pfr-orm/sqlite/utils.hpp>

namespace pfrorm::sqlite::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

} // namespace pfrorm::sqlite::detail
