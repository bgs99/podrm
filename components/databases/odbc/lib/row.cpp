#include <podrm/odbc/detail/entry.hpp>
#include <podrm/odbc/detail/row.hpp>

#include <fmt/core.h>

namespace podrm::odbc::detail {

Entry Row::get(const int column) const {
  if (column < 0 || column >= this->columnCount) {
    throw InvalidRowError{
        fmt::format("Column {} is out of range, result contains {} columns",
                    column, this->columnCount)};
  }

  return Entry{this->statement, column + 1};
}

} // namespace podrm::odbc::detail
