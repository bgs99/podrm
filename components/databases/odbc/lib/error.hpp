#pragma once

#include <string>

#include <sql.h>
#include <sqltypes.h>

namespace podrm::odbc {

std::string extractError(SQLHANDLE handle, SQLSMALLINT type);

inline std::string statementError(SQLHSTMT statement) {
  return extractError(statement, SQL_HANDLE_STMT);
}

} // namespace podrm::odbc
