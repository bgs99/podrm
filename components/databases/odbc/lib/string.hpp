#pragma once

#include <podrm/span.hpp>

#include <string_view>

#include <sqltypes.h>

namespace podrm::odbc::detail {

using String = span<SQLCHAR>;

String makeString(std::string_view str);

} // namespace podrm::odbc::detail
