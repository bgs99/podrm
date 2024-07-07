#pragma once

#include <podrm/metadata.hpp>
#include <podrm/postgres/detail/connection.hpp>

#include <string>
#include <utility>

namespace podrm::postgres {

class Database {
public:
  Database(const std::string &connectionStr)
      : Database(detail::Connection{connectionStr}) {}

  template <DatabaseEntity T> void createTable() {
    return this->connection.createTable(DatabaseEntityDescription<T>.value());
  }

  template <DatabaseEntity T> bool exists() {
    return this->connection.exists(DatabaseEntityDescription<T>.value());
  }

private:
  detail::Connection connection;

  explicit Database(detail::Connection &&connection)
      : connection(std::move(connection)) {}
};

} // namespace podrm::postgres
