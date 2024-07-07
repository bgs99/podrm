#include "error.hpp"

#include <podrm/odbc/detail/result.hpp>

#include <cassert>
#include <optional>
#include <stdexcept>
#include <utility>

#include <sql.h>
#include <sqltypes.h>

namespace podrm::odbc::detail {

Result::Result(Result::Statement statement) : statement(std::move(statement)) {
  this->nextRow();
}

bool Result::nextRow() {
  assert(this->statement.has_value());

  const int result = SQLFetch(this->statement->get());
  if (result == SQL_NO_DATA) {
    this->statement.reset();
    return false;
  }

  if (!SQL_SUCCEEDED(result)) {
    throw std::runtime_error{statementError(this->statement->get())};
  }

  SQLSMALLINT columns = 0;
  SQLNumResultCols(this->statement->get(), &columns);

  this->columnCount = columns;

  return true;
}

bool Result::valid() const { return this->statement.has_value(); }

} // namespace podrm::odbc::detail
