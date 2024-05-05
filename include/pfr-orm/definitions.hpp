#pragma once

#include "api.hpp"
#include "boost/pfr/core.hpp"
#include "boost/pfr/core_name.hpp"
#include "pfr-orm/detail/escaping.hpp"
#include "pfr-orm/detail/pfr.hpp"
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
  std::vector<FieldDescription> fields;
};

struct FieldDescription {
  std::string_view name;
  std::variant<PrimitiveFieldDescription, CompositeFieldDescription> field;
};

struct EntityDescription {
  std::string_view name;
  std::vector<FieldDescription> fields;
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

template <DatabasePrimitive Field>
FieldDescription getFieldDescriptionOfField(const std::string_view name) {
  return FieldDescription{
      .name{name},
      .field =
          PrimitiveFieldDescription{
              .nativeType = ValueRegistration<Field>::NativeType,
          },
  };
}

template <DatabaseComposite Field>
FieldDescription getFieldDescriptionOfField(const std::string_view name) {
  return FieldDescription{
      .name{name},
      .field =
          CompositeFieldDescription{
              .fields = getFieldDescriptions<Field>(),
          },
  };
}

template <detail::Reflectable T, std::size_t FieldsCount, std::size_t... Idx>
std::vector<FieldDescription>
getFieldDescriptions(std::array<std::string_view, FieldsCount> names,
                     std::index_sequence<Idx...>) {
  return {getFieldDescriptionOfField<boost::pfr::tuple_element_t<Idx, T>>(
      names[Idx])...};
}

template <detail::Reflectable T>
std::vector<FieldDescription> getFieldDescriptions() {
  const std::array names = boost::pfr::names_as_array<T>();
  return getFieldDescriptions<T>(std::move(names),
                                 std::make_index_sequence<std::size(names)>());
}

} // namespace detail

template <DatabaseEntity T>
EntityDescription DatabaseEntityDescription{
    .name = detail::TypeName<T>,
    .fields = detail::getFieldDescriptions<T>(),
};

} // namespace pfrorm
