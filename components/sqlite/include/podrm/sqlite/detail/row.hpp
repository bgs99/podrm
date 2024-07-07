#pragma once

#include <podrm/sqlite/detail/entry.hpp>

#include <stdexcept>

struct sqlite3_stmt;

namespace podrm::sqlite::detail {

class Row {
public:
  class InvalidRowError : public std::out_of_range {
  public:
    using std::out_of_range::out_of_range;
  };

  [[nodiscard]] int getColumnCount() const { return this->columnCount; }

  /// @throws InvalidRowError if the column is outside of the range
  [[nodiscard]] Entry get(int column) const;

private:
  sqlite3_stmt *statement;

  int columnCount;

  friend class Result;

  explicit Row(sqlite3_stmt *const statement, const int columnCount)
      : statement(statement), columnCount(columnCount) {}
};

} // namespace podrm::sqlite::detail
