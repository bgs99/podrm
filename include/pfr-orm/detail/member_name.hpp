#pragma once

#include <cstddef>
#include <string_view>

namespace pfrorm::detail {

template <const auto MemberPtr>
constexpr std::string_view wrappedMemberNameImpl() {
#ifdef __clang__
  return static_cast<const char *>(__PRETTY_FUNCTION__);
#elif defined(__GNUC__)
  return static_cast<const char *>(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
  return static_cast<const char *>(__FUNCSIG__);
#else
#error "Unsupported compiler"
#endif
}

template <const auto MemberPtr>
constexpr std::string_view WrappedMemberName =
    wrappedMemberNameImpl<MemberPtr>();

struct ExampleStruct {
  int example;
};

constexpr std::string_view ExampleMemberName = "ExampleStruct::example";

constexpr std::size_t WrappedMemberNamePrefixLength =
    wrappedMemberNameImpl<&ExampleStruct::example>().find(ExampleMemberName);

constexpr std::size_t WrappedMemberNameSuffixLength =
    wrappedMemberNameImpl<&ExampleStruct::example>().length() -
    WrappedMemberNamePrefixLength - ExampleMemberName.length();

template <const auto MemberPtr>
constexpr std::size_t MemberNameLength =
    WrappedMemberName<MemberPtr>.length() - WrappedMemberNamePrefixLength -
    WrappedMemberNameSuffixLength;

template <const auto MemberPtr>
constexpr std::string_view MemberName = WrappedMemberName<MemberPtr>.substr(
    WrappedMemberNamePrefixLength, MemberNameLength<MemberPtr>);

constexpr std::string_view simplifyMemberName(std::string_view name) {
  const auto namespaceStart = name.rfind(':');
  if (namespaceStart != std::string_view::npos) {
    name = name.substr(namespaceStart + 1);
  }

  return name;
}

template <const auto MemberPtr>
constexpr std::string_view SimpleMemberName =
    simplifyMemberName(MemberName<MemberPtr>);

} // namespace pfrorm::detail
