#pragma once

#include <pfr-orm/definitions.hpp>
#include <pfr-orm/sqlite/detail/operations.hpp>
#include <pfr-orm/sqlite/utils.hpp>

namespace pfrorm::sqlite {

template <DatabaseEntity T> void createTable(Connection &connection) {
  return detail::createTable(connection, DatabaseEntityDescription<T>);
}

template <typename T> bool exists(Connection &connection) {
  return detail::exists(connection, DatabaseEntityDescription<T>);
}

} // namespace pfrorm::sqlite
