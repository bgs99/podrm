#pragma once

#include <podrm/metadata.hpp>
#include <podrm/odbc/detail/result.hpp>
#include <podrm/span.hpp>

namespace podrm::odbc::detail {

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

} // namespace podrm::odbc::detail
