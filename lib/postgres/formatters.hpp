#pragma once

#include <pfr-orm/postgres/utils.hpp>

#include <string_view>

#include <fmt/core.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<pfrorm::postgres::Str> : formatter<std::string_view> {
public:
  template <typename FormatContext>
  constexpr auto format(const pfrorm::postgres::Str &str,
                        FormatContext &ctx) const -> decltype(ctx.out()) {
    return formatter<std::string_view>::format(str.view(), ctx);
  }
};
