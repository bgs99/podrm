#pragma once

#include <pfr-orm/detail/pfr.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>

#include <boost/pfr/core_name.hpp>

namespace pfrorm {

template <typename T> class FieldDescriptor;

namespace detail {

/// @todo get rid of the field name requirement, use field member ptr
template <Reflectable T>
constexpr FieldDescriptor<T> getFieldDescriptor(const auto T::*const /*field*/,
                                                std::string_view fieldName) {
  const std::array fieldNames = boost::pfr::names_as_array<T>();
  const auto it = std::find(fieldNames.cbegin(), fieldNames.cend(), fieldName);
  if (it == fieldNames.cend()) {
    throw std::runtime_error("Type does not have a field " +
                             std::string{fieldName});
  }

  return FieldDescriptor<T>{static_cast<std::size_t>(it - fieldNames.cbegin())};
}

} // namespace detail
} // namespace pfrorm
