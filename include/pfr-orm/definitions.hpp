#pragma once

#include "api.hpp"
#include "boost/pfr/core.hpp"
#include "boost/pfr/core_name.hpp"
#include "boost/pfr/tuple_size.hpp"
#include "pfr-orm/detail/pfr.hpp"
#include "pfr-orm/detail/span.hpp"
#include "pfr-orm/detail/type_name.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace pfrorm {

struct PrimitiveFieldDescription {
  std::string_view nativeType;

  friend constexpr bool operator==(PrimitiveFieldDescription,
                                   PrimitiveFieldDescription) = default;
};

struct FieldDescription;

struct CompositeFieldDescription {
  detail::span<const FieldDescription> fields;
};

struct FieldDescription {
  std::string_view name;
  std::variant<PrimitiveFieldDescription, CompositeFieldDescription> field;

  friend constexpr bool operator==(FieldDescription,
                                   FieldDescription) = default;
};

constexpr bool operator==(CompositeFieldDescription lhs,
                          CompositeFieldDescription rhs) {
  if (lhs.fields.size() != rhs.fields.size()) {
    return false;
  }

  for (std::size_t i = 0; i < lhs.fields.size(); ++i) {
    if (lhs.fields[i] != rhs.fields[i]) {
      return false;
    }
  }

  return true;
}

struct EntityDescription {
  std::string_view name;
  detail::span<const FieldDescription> fields;
};

template <typename T>
concept DatabaseEntity = detail::Reflectable<T> &&
  requires()
{
  typename EntityRegistration<T>;
};

template <typename T>
concept DatabaseComposite = detail::Reflectable<T> &&
  requires()
{
  typename CompositeRegistration<T>;
};

template <typename T>
concept DatabasePrimitive = requires() { ValueRegistration<T>::NativeType; };

namespace detail {

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
getFieldDescriptions();

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
    FieldDescriptions = getFieldDescriptions<T>();

template <DatabasePrimitive Field>
constexpr FieldDescription
getFieldDescriptionOfField(const std::string_view name) {
  return FieldDescription{
      .name{name},
      .field =
          PrimitiveFieldDescription{
              .nativeType = ValueRegistration<Field>::NativeType,
          },
  };
}

template <DatabaseComposite Field>
  requires(not DatabasePrimitive<Field>)
constexpr FieldDescription
getFieldDescriptionOfField(const std::string_view name) {
  return FieldDescription{
      .name{name},
      .field =
          CompositeFieldDescription{
              .fields = FieldDescriptions<Field>,
          },
  };
}

template <detail::Reflectable T, std::size_t FieldsCount, std::size_t... Idx>
constexpr std::array<FieldDescription, FieldsCount>
getFieldDescriptions(std::array<std::string_view, FieldsCount> names,
                     std::index_sequence<Idx...>) {
  return {getFieldDescriptionOfField<boost::pfr::tuple_element_t<Idx, T>>(
      names[Idx])...};
}

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
getFieldDescriptions() {
  const std::array names = boost::pfr::names_as_array<T>();
  return getFieldDescriptions<T>(std::move(names),
                                 std::make_index_sequence<std::size(names)>());
}

} // namespace detail

template <DatabaseEntity T>
constexpr EntityDescription DatabaseEntityDescription{
    .name = detail::SimpleTypeName<T>,
    .fields = detail::FieldDescriptions<T>,
};

} // namespace pfrorm
