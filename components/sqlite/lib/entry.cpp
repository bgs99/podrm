#include <podrm/span.hpp>
#include <podrm/sqlite/detail/entry.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include <sqlite3.h>

namespace podrm::sqlite::detail {

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

} // namespace podrm::sqlite::detail
