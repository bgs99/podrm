#pragma once

#ifdef PFR_ORM_USE_GSL_SPAN
#include <gsl/span>
#include <gsl/span_ext>
#else
#include <span>
#endif

namespace podrm::detail {

#ifdef PFR_ORM_USE_GSL_SPAN
template <typename T> using span = gsl::span<T>;
#else
template <typename T> using span = std::span<T>;
#endif

} // namespace podrm::detail
