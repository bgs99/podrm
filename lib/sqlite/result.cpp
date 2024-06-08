#include <podrm/sqlite/detail/result.hpp>

#include <cassert>
#include <optional>
#include <stdexcept>
#include <utility>

#include <sqlite3.h>

namespace podrm::sqlite::detail {

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

bool Result::valid() const { return this->statement.has_value(); }

} // namespace podrm::sqlite::detail
