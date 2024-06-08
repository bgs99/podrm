#pragma once

#include <podrm/postgres/utils.hpp>
#include <podrm/reflection.hpp>

namespace podrm::postgres::detail {

void createTable(Connection &connection, const EntityDescription &entity);

bool exists(Connection &connection, const EntityDescription &entity);

} // namespace podrm::postgres::detail
