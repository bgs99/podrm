#pragma once

#include <string_view>

struct pg_result;

namespace podrm::postgres::detail {

class Result {
public:
  Result(pg_result *result) : result(result) {}
  ~Result();

  [[nodiscard]] int status() const;
  [[nodiscard]] std::string_view value(int row, int column) const;

  Result(const Result &) = delete;
  Result(Result &&) noexcept;
  Result &operator=(const Result &) = delete;
  Result &operator=(Result &&) = delete;

private:
  pg_result *result;
};

} // namespace podrm::postgres::detail
