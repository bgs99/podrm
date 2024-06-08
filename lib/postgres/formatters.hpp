#pragma once

#include <podrm/postgres/detail/connection.hpp>

#include <string_view>

#include <fmt/core.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<podrm::postgres::detail::Str>
    : formatter<std::string_view> {
public:
  template <typename FormatContext>
  constexpr auto format(const podrm::postgres::detail::Str &str,
                        FormatContext &ctx) const -> decltype(ctx.out()) {
    return formatter<std::string_view>::format(str.view(), ctx);
  }
};
