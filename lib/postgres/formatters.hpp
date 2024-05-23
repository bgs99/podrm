#pragma once

#include <podrm/postgres/utils.hpp>

#include <string_view>

#include <fmt/core.h>
#include <fmt/format.h>

template <>
struct fmt::formatter<podrm::postgres::Str> : formatter<std::string_view> {
public:
  template <typename FormatContext>
  constexpr auto format(const podrm::postgres::Str &str,
                        FormatContext &ctx) const -> decltype(ctx.out()) {
    return formatter<std::string_view>::format(str.view(), ctx);
  }
};
