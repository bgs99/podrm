#pragma once

#include <podrm/odbc/detail/entry.hpp>

#include <stdexcept>

namespace podrm::odbc::detail {

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
  void *statement;

  int columnCount;

  friend class Result;

  explicit Row(void *const statement, const int columnCount)
      : statement(statement), columnCount(columnCount) {}
};

} // namespace podrm::odbc::detail
