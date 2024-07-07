#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace podrm::odbc::detail {

class Entry {
public:
  [[nodiscard]] std::string text() const;

  [[nodiscard]] std::int64_t bigint() const;

  [[nodiscard]] double real() const;

  [[nodiscard]] bool boolean() const;

  [[nodiscard]] std::vector<std::byte> bytes() const;

private:
  void *statement;

  int column;

  friend class Row;

  explicit Entry(void *statement, int column);
};

} // namespace podrm::odbc::detail
