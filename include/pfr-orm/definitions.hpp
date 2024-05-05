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

#include <gsl/span>

namespace pfrorm {

struct PrimitiveFieldDescription {
  std::string_view nativeType;
};

struct FieldDescription;

struct CompositeFieldDescription {
  detail::span<const FieldDescription> fields;
};

struct FieldDescription {
  std::string_view name;
  std::variant<PrimitiveFieldDescription, CompositeFieldDescription> field;
};

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
concept DatabasePrimitive = requires() { typename ValueRegistration<T>; };

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
    .name = detail::TypeName<T>,
    .fields = detail::FieldDescriptions<T>,
};

} // namespace pfrorm
