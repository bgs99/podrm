#pragma once

#include <podrm/definitions.hpp>
#include <podrm/sqlite/utils.hpp>

namespace podrm::sqlite::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

} // namespace podrm::sqlite::detail
