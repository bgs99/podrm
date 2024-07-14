#include <podrm/odbc/environment.hpp>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

namespace podrm::odbc {

namespace {

SQLHENV createEnvironment() {
  SQLHENV environment = nullptr;
  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment);
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): ODBC API
  SQLSetEnvAttr(
      environment, SQL_ATTR_ODBC_VERSION,
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): ODBC API
      reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);

  return environment;
}

void freeEnvironment(SQLHENV environment) { SQLFreeEnv(environment); }

} // namespace

Environment::Environment() : Environment(createEnvironment()) {}

Environment Environment::fromRaw(void *environment) {
  return Environment{environment};
}

Environment::Environment(void *environment)
    : impl(environment, freeEnvironment) {}

} // namespace podrm::odbc
