#pragma once

#include <string_view>

namespace podrm::postgres::detail {

class Str {
public:
  Str(char *str) : str(str) {}
  ~Str();

  [[nodiscard]] std::string_view view() const { return str; }
  operator std::string_view() const { return str; }

  Str(const Str &) = delete;
  Str(Str &&) = delete;
  Str &operator=(const Str &) = delete;
  Str &operator=(Str &&) = delete;

private:
  char *str;
};

} // namespace podrm::postgres::detail
