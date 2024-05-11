#include <pfr-orm/postgres/utils.hpp>

#include <stdexcept>
#include <string>
#include <string_view>

#include <fmt/core.h>
#include <libpq-fe.h>

namespace pfrorm::postgres {

Str::~Str() { PQfreemem(this->str); }

Result::Result(Result &&other) noexcept : result(other.result) {
  other.result = nullptr;
}

Result::~Result() {
  if (this->result != nullptr) {
    PQclear(this->result);
  }
}

[[nodiscard]] int Result::status() const {
  return PQresultStatus(this->result);
}

std::string_view Result::value(const int row, const int column) const {
  return PQgetvalue(this->result, row, column);
}

Connection::Connection(const std::string &connectionStr)
    : connection(PQconnectdb(connectionStr.c_str())) {
  if (PQstatus(this->connection) != CONNECTION_OK) {
    throw std::runtime_error{fmt::format("Failed to connect to db: {}",
                                         PQerrorMessage(this->connection))};
  }
}

Connection::~Connection() { PQfinish(this->connection); }

Str Connection::escapeIdentifier(const std::string_view identifier) const {
  return Str{PQescapeIdentifier(this->connection, identifier.data(),
                                identifier.size())};
}

Result Connection::execute(const std::string &statement) {
  Result result{PQexec(this->connection, statement.c_str())};
  if (result.status() != PGRES_COMMAND_OK) {
    throw std::runtime_error{
        fmt::format("Error when executing a statement: {}",
                    PQerrorMessage(this->connection)),
    };
  }
  return result;
}

Result Connection::query(const std::string &statement) {
  Result result{PQexec(this->connection, statement.c_str())};
  if (result.status() != PGRES_TUPLES_OK) {
    throw std::runtime_error{
        fmt::format("Error when executing a query: {}",
                    PQerrorMessage(this->connection)),
    };
  }
  return result;
}

} // namespace pfrorm::postgres
