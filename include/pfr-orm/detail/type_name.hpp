#pragma once

#include <cstddef>
#include <string_view>

namespace pfrorm::detail {

template <typename T> constexpr std::string_view wrappedTypeNameImpl() {
#ifdef __clang__
  return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
  return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
  return __FUNCSIG__;
#else
#error "Unsupported compiler"
#endif
}

template <typename T>
constexpr std::string_view WrappedTypeName = wrappedTypeNameImpl<T>();

constexpr std::string_view voidTypeName = "void";

constexpr std::size_t WrappedTypeNamePrefixLength =
    wrappedTypeNameImpl<void>().find(voidTypeName);

constexpr std::size_t WrappedTypeNameSuffixLength =
    wrappedTypeNameImpl<void>().length() - WrappedTypeNamePrefixLength -
    voidTypeName.length();

template <typename T>
constexpr std::size_t TypeNameLength =
    WrappedTypeName<T>.length() - WrappedTypeNamePrefixLength -
    WrappedTypeNameSuffixLength;

template <typename T>
constexpr std::string_view TypeName =
    WrappedTypeName<T>.substr(WrappedTypeNamePrefixLength, TypeNameLength<T>);

constexpr std::string_view simplifyTypeName(std::string_view name) {
  const auto namespaceStart = name.rfind(':');
  if (namespaceStart != std::string_view::npos) {
    name = name.substr(namespaceStart + 1);
  }

  return name;
}

template <typename T>
constexpr std::string_view SimpleTypeName = simplifyTypeName(TypeName<T>);

} // namespace pfrorm::detail
