#pragma once

#include <podrm/reflection.hpp>
#include <podrm/span.hpp>
#include <podrm/sqlite/detail/result.hpp>

namespace podrm::sqlite::detail {

class Cursor {
public:
  Cursor(Result result, span<const FieldDescription> description);

  /// @param[out] data data to be initialized
  [[nodiscard]] bool extract(void *data) const;

  bool nextRow();

  [[nodiscard]] bool valid() const;

private:
  Result result;

  span<const FieldDescription> description;
};

} // namespace podrm::sqlite::detail
