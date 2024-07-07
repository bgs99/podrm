#pragma once

#include "podrm/odbc/environment.hpp"

#include <podrm/metadata.hpp>
#include <podrm/odbc/cursor.hpp>
#include <podrm/odbc/detail/connection.hpp>

#include <optional>
#include <string_view>
#include <utility>

namespace podrm::odbc {

class Database {
public:
  //---------------- Constructors ------------------//

  static Database fromRaw(void *connection) {
    return Database{detail::Connection::fromRaw(connection)};
  }

  static Database
  fromConnectionString(Environment &environment,
                       const std::string_view connectionString) {
    return Database{detail::Connection::fromConnectionString(environment,
                                                             connectionString)};
  }

  //---------------- Operations ------------------//

  template <DatabaseEntity T> void createTable() {
    return this->connection.createTable(DatabaseEntityDescription<T>.value());
  }

  template <typename T> bool exists() {
    return this->connection.exists(DatabaseEntityDescription<T>.value());
  }

  template <DatabaseEntity Entity> void persist(Entity &entity) {
    return this->connection.persist(DatabaseEntityDescription<Entity>.value(),
                                    &entity);
  }

  template <DatabaseEntity Entity>
  std::optional<Entity> find(const PrimaryKeyType<Entity> &key) {
    Entity result;
    if (!this->connection.find(DatabaseEntityDescription<Entity>.value(), key,
                               &result)) {
      return std::nullopt;
    }

    return result;
  }

  template <DatabaseEntity Entity>
  void erase(const PrimaryKeyType<Entity> &key) {
    this->connection.erase(DatabaseEntityDescription<Entity>.value(), key);
  }

  template <DatabaseEntity Entity> void update(const Entity &entity) {
    this->connection.update(DatabaseEntityDescription<Entity>.value(), &entity);
  }

  template <DatabaseEntity Entity> Cursor<Entity> iterate() {
    return Cursor<Entity>{
        this->connection.iterate(DatabaseEntityDescription<Entity>.value()),
    };
  }

private:
  detail::Connection connection;

  explicit Database(detail::Connection &&connection)
      : connection(std::move(connection)) {}
};

} // namespace podrm::odbc
