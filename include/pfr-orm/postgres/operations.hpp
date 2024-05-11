#pragma once

#include <pfr-orm/definitions.hpp>
#include <pfr-orm/detail/operations.hpp>
#include <pfr-orm/postgres/utils.hpp>

namespace pfrorm::postgres {

template <DatabaseEntity T> void createTable(Connection &connection) {
  return detail::createTable(connection, DatabaseEntityDescription<T>);
}

template <typename T> bool exists(Connection &connection) {
  return detail::exists(connection, DatabaseEntityDescription<T>);
}

} // namespace pfrorm::postgres
