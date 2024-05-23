#pragma once

#include <podrm/definitions.hpp>
#include <podrm/postgres/detail/operations.hpp>
#include <podrm/postgres/utils.hpp>

namespace podrm::postgres {

template <DatabaseEntity T> void createTable(Connection &connection) {
  return detail::createTable(connection, DatabaseEntityDescription<T>);
}

template <typename T> bool exists(Connection &connection) {
  return detail::exists(connection, DatabaseEntityDescription<T>);
}

} // namespace podrm::postgres
