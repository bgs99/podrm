#pragma once

#include <podrm/span.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>

struct sqlite3_stmt;

namespace podrm::sqlite::detail {

class Entry {
public:
  [[nodiscard]] std::string_view text() const;

  [[nodiscard]] std::int64_t bigint() const;

  [[nodiscard]] double real() const;

  [[nodiscard]] bool boolean() const;

  [[nodiscard]] span<const std::byte> bytes() const;

private:
  sqlite3_stmt *statement;

  int column;

  friend class Row;

  explicit Entry(sqlite3_stmt *statement, int column);
};

} // namespace podrm::sqlite::detail
