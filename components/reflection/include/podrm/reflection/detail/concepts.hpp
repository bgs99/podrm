#pragma once

#include <type_traits>

namespace podrm::detail {

template <typename T, typename Other>
concept same_as = std::is_same_v<T, Other>;

template <typename From, typename To>
concept convertible_to = std::is_convertible_v<From, To>;

} // namespace podrm::detail
