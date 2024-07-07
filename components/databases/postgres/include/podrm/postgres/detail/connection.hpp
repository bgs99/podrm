#pragma once

#include <podrm/metadata.hpp>
#include <podrm/postgres/detail/result.hpp>
#include <podrm/postgres/detail/str.hpp>

#include <string>
#include <string_view>

struct pg_conn;

namespace podrm::postgres::detail {

class Connection {
public:
  Connection(const std::string &connectionStr);
  ~Connection();

  void createTable(const EntityDescription &entity);

  bool exists(const EntityDescription &entity);

  Connection(const Connection &) = delete;
  Connection(Connection &&) noexcept;
  Connection &operator=(const Connection &) = delete;
  Connection &operator=(Connection &&) = delete;

  [[nodiscard]] Str escapeIdentifier(std::string_view identifier) const;

private:
  pg_conn *connection;

  Result execute(const std::string &statement);

  Result query(const std::string &statement);
};

} // namespace podrm::postgres::detail
