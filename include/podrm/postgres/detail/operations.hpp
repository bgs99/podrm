#pragma once

#include <podrm/definitions.hpp>
#include <podrm/postgres/utils.hpp>

namespace podrm::postgres::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

} // namespace podrm::postgres::detail
