#pragma once

#include <cstddef>
#include <string_view>

namespace pfrorm::detail {

template <typename T> constexpr std::string_view wrappedTypeNameImpl() {
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

template <typename T>
constexpr std::string_view WrappedTypeName = wrappedTypeNameImpl<T>();

constexpr std::string_view VoidTypeName = "void";

constexpr std::size_t WrappedTypeNamePrefixLength =
    wrappedTypeNameImpl<void>().find(VoidTypeName);

constexpr std::size_t WrappedTypeNameSuffixLength =
    wrappedTypeNameImpl<void>().length() - WrappedTypeNamePrefixLength -
    VoidTypeName.length();

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
