#include "error.hpp"

#include <array>
#include <cstddef>
#include <string>

#include <sql.h>
#include <sqltypes.h>

namespace podrm::odbc {

std::string extractError(SQLHANDLE handle, const SQLSMALLINT type) {
  SQLINTEGER native = 0;

  constexpr static std::size_t MaxStateSize = 7;

  std::array<SQLCHAR, MaxStateSize> state = {};

  constexpr static std::size_t MaxMessageSize = 256;

  std::array<SQLCHAR, MaxMessageSize> text = {};
  SQLSMALLINT length = 0;

  SQLGetDiagRec(type, handle, 1, state.data(), &native, text.data(),
                text.size(), &length);

  return std::string{
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): safe
      reinterpret_cast<char *>(text.data()),
      static_cast<std::size_t>(length),
  };
}
} // namespace podrm::odbc
