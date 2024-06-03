#pragma once

#include <podrm/definitions.hpp>
#include <podrm/sqlite/detail/operations.hpp>
#include <podrm/sqlite/utils.hpp>

#include <optional>

namespace podrm::sqlite {

template <DatabaseEntity T> void createTable(Connection &connection) {
  return detail::createTable(connection, DatabaseEntityDescription<T>);
}

template <typename T> bool exists(Connection &connection) {
  return detail::exists(connection, DatabaseEntityDescription<T>);
}

template <DatabaseEntity Entity>
void persist(Connection &connection, Entity &entity) {
  return detail::persist(connection, DatabaseEntityDescription<Entity>,
                         &entity);
}

template <DatabaseEntity Entity>
std::optional<Entity> find(Connection &connection,
                           const PrimaryKeyType<Entity> &key) {
  Entity result;
  if (!detail::find(connection, DatabaseEntityDescription<Entity>, key,
                    &result)) {
    return std::nullopt;
  }

  return result;
}

template <DatabaseEntity Entity>
void erase(Connection &connection, const PrimaryKeyType<Entity> &key) {
  detail::erase(connection, DatabaseEntityDescription<Entity>, key);
}

template <DatabaseEntity Entity>
void update(Connection &connection, const Entity &entity) {
  detail::update(connection, DatabaseEntityDescription<Entity>, &entity);
}

} // namespace podrm::sqlite
