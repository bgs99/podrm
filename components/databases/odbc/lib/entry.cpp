#include <podrm/odbc/detail/entry.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

namespace podrm::odbc::detail {

std::string Entry::text() const {
  SQLLEN dataLen = 0;
  std::string result;
  SQLGetData(this->statement, this->column, SQL_C_CHAR, result.data(), 0,
             &dataLen);
  result.resize(dataLen);
  SQLGetData(this->statement, this->column, SQL_C_CHAR, result.data(),
             dataLen + 1, nullptr);

  return result;
}

std::int64_t Entry::bigint() const {
  std::int64_t result = 0;
  SQLGetData(this->statement, this->column, SQL_C_SBIGINT, &result, 0, nullptr);
  return result;
}

double Entry::real() const {
  double result = 0;
  SQLGetData(this->statement, this->column, SQL_C_DOUBLE, &result, 0, nullptr);
  return result;
}

bool Entry::boolean() const {
  bool result = false;
  SQLGetData(this->statement, this->column, SQL_C_BIT, &result, 0, nullptr);
  return result;
}

std::vector<std::byte> Entry::bytes() const {
  SQLLEN dataLen = 0;
  std::vector<std::byte> result;
  SQLGetData(this->statement, this->column, SQL_C_BINARY, result.data(), 0,
             &dataLen);
  result.resize(dataLen);
  SQLGetData(this->statement, this->column, SQL_C_BINARY, result.data(),
             dataLen, nullptr);

  return result;
}

Entry::Entry(SQLHSTMT statement, const int column)
    : statement(statement), column(column) {}

} // namespace podrm::odbc::detail
