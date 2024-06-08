#include "../detail/multilambda.hpp"

#include <podrm/api.hpp>
#include <podrm/span.hpp>
#include <podrm/sqlite/utils.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include <fmt/core.h>
#include <sqlite3.h>

namespace podrm::sqlite {

namespace {

using Statement =
    std::unique_ptr<sqlite3_stmt, decltype([](sqlite3_stmt *statement) {
                      sqlite3_finalize(statement);
                    })>;

Statement createStatement(sqlite3 &connection,
                          const std::string_view statement) {
  sqlite3_stmt *stmt = nullptr;
  const int result =
      sqlite3_prepare_v3(&connection, statement.data(),
                         static_cast<int>(statement.size()), 0, &stmt, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errmsg(&connection)};
  }

  return Statement{stmt};
}

void bindArg(const Statement &statement, const int pos, const AsImage &value) {
  const auto bindInt = [&statement, pos](const std::int64_t value) {
    sqlite3_bind_int64(statement.get(), pos + 1, value);
  };
  const auto bindUInt = [&statement, pos](const std::uint64_t value) {
    if (value > std::numeric_limits<std::int64_t>::max()) {
      throw std::invalid_argument{"Unsigned integer too big"};
    }
    sqlite3_bind_int64(statement.get(), pos + 1,
                       static_cast<std::int64_t>(value));
  };
  const auto bindBlob = [&statement, pos](const span<const std::byte> blob) {
    sqlite3_bind_blob64(statement.get(), pos, blob.data(), blob.size(),
                        SQLITE_STATIC);
  };
  const auto bindDouble = [&statement, pos](const double value) {
    sqlite3_bind_double(statement.get(), pos + 1, value);
  };
  const auto bindText = [&statement, pos](const std::string_view text) {
    sqlite3_bind_text64(statement.get(), pos + 1, text.data(), text.size(),
                        SQLITE_STATIC, SQLITE_UTF8);
  };
  const auto bindBool = [&statement, pos](const bool value) {
    sqlite3_bind_int(statement.get(), pos + 1, value ? 1 : 0);
  };

  std::visit(detail::MultiLambda{bindBlob, bindDouble, bindText, bindInt,
                                 bindUInt, bindBool},
             value);
}

} // namespace

std::string_view Entry::text() const {
  return {
      // Required and safe
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast):
      reinterpret_cast<const char *>(
          sqlite3_column_text(this->statement, this->column)),
      static_cast<std::size_t>(
          sqlite3_column_bytes(this->statement, this->column)),
  };
}

std::int64_t Entry::bigint() const {

  return sqlite3_column_int64(this->statement, this->column);
}

double Entry::real() const {
  return sqlite3_column_double(this->statement, this->column);
}

bool Entry::boolean() const { return this->bigint() != 0; }

span<const std::byte> Entry::bytes() const {
  return {
      static_cast<const std::byte *>(
          sqlite3_column_blob(this->statement, this->column)),
      static_cast<std::size_t>(
          sqlite3_column_bytes(this->statement, this->column)),
  };
}

Entry::Entry(sqlite3_stmt *const statement, const int column)
    : statement(statement), column(column) {}

Entry Row::get(const int column) const {
  if (column < 0 || column > this->columnCount) {
    throw InvalidRowError{
        fmt::format("Column {} is out of range, result contains {} columns",
                    column, this->columnCount)};
  }

  return Entry{this->statement, column};
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

Connection::Connection(sqlite3 &connection)
    : connection(&connection, &sqlite3_close_v2) {
  this->execute("PRAGMA foreign_keys = ON");
}

Connection Connection::fromRaw(sqlite3 &connection) {
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

  return Connection{*connection};
}

Connection Connection::inFile(const std::filesystem::path &path) {
  sqlite3 *connection = nullptr;
  const int result =
      sqlite3_open_v2(path.string().c_str(), &connection,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (result != SQLITE_OK) {
    throw std::runtime_error{sqlite3_errstr(result)};
  }

  return Connection{*connection};
}

std::uint64_t Connection::execute(const std::string_view statement,
                                  const span<const AsImage> args) {
  std::unique_lock lock{this->mutex};

  const Statement stmt = createStatement(*this->connection, statement);

  for (int i = 0; i < args.size(); ++i) {
    bindArg(stmt, i, args[i]);
  }

  const int executeResult = sqlite3_step(stmt.get());
  if (executeResult != SQLITE_DONE) {
    throw std::runtime_error{sqlite3_errmsg(this->connection.get())};
  }

  return sqlite3_changes64(this->connection.get());
}

Result Connection::query(const std::string_view statement,
                         const span<const AsImage> args) {
  std::unique_lock lock{this->mutex};

  Statement stmt = createStatement(*this->connection, statement);

  for (int i = 0; i < args.size(); ++i) {
    bindArg(stmt, i, args[i]);
  }

  return Result{{stmt.release(), &sqlite3_finalize}};
}

} // namespace podrm::sqlite
