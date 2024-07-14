#include "string.hpp"

#include <string_view>

#include <sqltypes.h>

namespace podrm::odbc::detail {

String makeString(const std::string_view str) {
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast): does not change
  auto *const data = const_cast<SQLCHAR *>(
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): safe
      reinterpret_cast<const SQLCHAR *>(str.data()));

  return {data, str.size()};
}

} // namespace podrm::odbc::detail
