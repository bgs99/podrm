#include <podrm/sqlite/detail/entry.hpp>
#include <podrm/sqlite/detail/row.hpp>

#include <fmt/core.h>

namespace podrm::sqlite::detail {

Entry Row::get(const int column) const {
  if (column < 0 || column > this->columnCount) {
    throw InvalidRowError{
        fmt::format("Column {} is out of range, result contains {} columns",
                    column, this->columnCount)};
  }

  return Entry{this->statement, column};
}

} // namespace podrm::sqlite::detail
