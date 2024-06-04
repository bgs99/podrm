#pragma once

#include <cstddef>

#ifdef PFR_ORM_USE_GSL_SPAN
#include <gsl/span>
#include <gsl/span_ext>
#else
#include <span>
#endif

namespace podrm {

#ifdef PFR_ORM_USE_GSL_SPAN
template <typename T, std::size_t Extent = gsl::dynamic_extent>
using span = gsl::span<T, Extent>;
#else
template <typename T, std::size_t Extent = std::dynamic_extent>
using span = std::span<T, Extent>;
#endif

} // namespace podrm
