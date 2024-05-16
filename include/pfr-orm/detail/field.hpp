#pragma once

#include <pfr-orm/detail/member_name.hpp>
#include <pfr-orm/detail/pfr.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <boost/pfr/core_name.hpp>

namespace pfrorm::detail {

template <Reflectable T, const auto MemberPtr>
constexpr std::size_t getFieldIndex() {
  const std::string_view fieldName = SimpleMemberName<MemberPtr>;
  const std::array fieldNames = boost::pfr::names_as_array<T>();
  const auto it = std::find(fieldNames.cbegin(), fieldNames.cend(), fieldName);
  if (it == fieldNames.cend()) {
    throw std::runtime_error("Type does not have a field " +
                             std::string{fieldName});
  }

  return static_cast<std::size_t>(it - fieldNames.cbegin());
}

template <auto MemberPtr> struct MemberPtrClassImpl {};

template <typename T, auto T::*MemberPtr> struct MemberPtrClassImpl<MemberPtr> {
  using Type = T;
};

template <typename T, const auto T::*MemberPtr>
struct MemberPtrClassImpl<MemberPtr> {
  using Type = T;
};

template <auto MemberPtr>
  requires(std::is_member_pointer_v<decltype(MemberPtr)>)
using MemberPtrClass = typename MemberPtrClassImpl<MemberPtr>::Type;

} // namespace pfrorm::detail
