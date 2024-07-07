#include <podrm/postgres/detail/result.hpp>

#include <string_view>

#include <libpq-fe.h>

namespace podrm::postgres::detail {

Result::Result(Result &&other) noexcept : result(other.result) {
  other.result = nullptr;
}

Result::~Result() {
  if (this->result != nullptr) {
    PQclear(this->result);
  }
}

[[nodiscard]] int Result::status() const {
  return PQresultStatus(this->result);
}

std::string_view Result::value(const int row, const int column) const {
  return PQgetvalue(this->result, row, column);
}

} // namespace podrm::postgres::detail
