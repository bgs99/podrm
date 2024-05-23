#pragma once

#include <boost/pfr/traits.hpp>

namespace podrm::detail {

template <typename T>
concept Reflectable = boost::pfr::is_implicitly_reflectable_v<T, void>;

} // namespace podrm::detail
