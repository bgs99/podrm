#include <pfr-orm/sqlite/utils.hpp>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include <fmt/core.h>
#include <sqlite3.h>

namespace pfrorm::sqlite {

namespace {

using Statement =
    std::unique_ptr<sqlite3_stmt, decltype([](sqlite3_stmt *statement) {
                      sqlite3_finalize(statement);
                    })>;

Statement createStatement(sqlite3 *const connection,
                          const std::string_view statement) {
  sqlite3_stmt *stmt = nullptr;
  const int result =
      sqlite3_prepare_v3(connection, statement.data(),
                         static_cast<int>(statement.size()), 0, &stmt, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errmsg(connection)};
  }

  return Statement{stmt};
}

} // namespace

std::string_view Row::text(const int column) const {
  if (column < 0 || column > this->columnCount) {
    throw InvalidRowError{
        fmt::format("Column {} is out of range, result contains {} columns",
                    column, this->columnCount)};
  }

  // Required and safe
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast):
  return reinterpret_cast<const char *>(
      sqlite3_column_text(this->statement, column));
}

[[nodiscard]] std::int64_t Row::bigint(int column) const {
  if (column < 0 || column > this->columnCount) {
    throw InvalidRowError{
        fmt::format("Column {} is out of range, result contains {} columns",
                    column, this->columnCount)};
  }

  return sqlite3_column_int64(this->statement, column);
}

[[nodiscard]] bool Row::boolean(int column) const {
  return this->bigint(column) != 0;
}

Result::Result(Result::Statement statement) : statement(std::move(statement)) {
  this->nextRow();
}

bool Result::nextRow() {
  assert(this->statement.has_value());

  const int result = sqlite3_step(this->statement->get());
  if (result == SQLITE_DONE) {
    this->statement.reset();
    return false;
  }

  if (result != SQLITE_ROW) {
    throw std::runtime_error{
        sqlite3_errmsg(sqlite3_db_handle(this->statement->get())),
    };
  }

  this->columnCount = sqlite3_data_count(this->statement->get());

  return true;
}

Connection::Connection(sqlite3 *const connection)
    : connection(connection, &sqlite3_close_v2) {}

Connection Connection::fromRaw(sqlite3 *const connection) {
  return Connection{connection};
}

Connection Connection::inMemory(const char *const name) {
  sqlite3 *connection = nullptr;
  const int result = sqlite3_open_v2(
      name, &connection,
      SQLITE_OPEN_MEMORY | SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errstr(result)};
  }

  return Connection{connection};
}

Connection Connection::inFile(const std::filesystem::path &path) {
  sqlite3 *connection = nullptr;
  const int result =
      sqlite3_open_v2(path.string().c_str(), &connection,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errstr(result)};
  }

  return Connection{connection};
}

void Connection::execute(const std::string_view statement) {
  Statement stmt = createStatement(this->connection.get(), statement);

  const int executeResult = sqlite3_step(stmt.get());
  if (executeResult != SQLITE_DONE) {
    throw std::runtime_error{sqlite3_errmsg(this->connection.get())};
  }
}

Result Connection::query(const std::string_view statement) {
  Statement stmt = createStatement(this->connection.get(), statement);

  return Result{{stmt.release(), &sqlite3_finalize}};
}

} // namespace pfrorm::sqlite
