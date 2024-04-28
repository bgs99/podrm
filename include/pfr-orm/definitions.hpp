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
#include <vector>

#include <gsl/span>

namespace pfrorm {

struct FieldDescription {
  std::string_view nativeType;
  std::string name;
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

inline static std::vector<FieldDescription>
addPrefixToFields(std::vector<FieldDescription> fields,
                  const std::string_view prefix) {
  for (FieldDescription &field : fields) {
    field.name = addIdentifierPrefix(prefix, field.name);
  }
  return fields;
}

template <DatabasePrimitive Field>
std::vector<FieldDescription>
getFieldDescriptionsOfField(const std::string_view name) {
  return {
      FieldDescription{
          .nativeType = ValueRegistration<Field>::NativeType,
          .name{name},
      },
  };
}

template <DatabaseComposite Field>
std::vector<FieldDescription>
getFieldDescriptionsOfField(const std::string_view name) {
  return addPrefixToFields(getFieldDescriptions<Field>(), name);
}

template <detail::Reflectable T, std::size_t FieldsCount, std::size_t... Idx>
std::vector<FieldDescription>
getFieldDescriptions(std::array<std::string_view, FieldsCount> names,
                     std::index_sequence<Idx...>) {
  std::array<std::vector<FieldDescription>, FieldsCount> fields = std::array{
      getFieldDescriptionsOfField<boost::pfr::tuple_element_t<Idx, T>>(
          names[Idx])...};

  const std::size_t totalFieldsCount = (fields[Idx].size() + ...);

  std::vector<FieldDescription> result;
  result.reserve(totalFieldsCount);

  for (std::vector<FieldDescription> &fieldVec : fields) {
    for (FieldDescription &field : fieldVec) {
      result.emplace_back(std::move(field));
    }
  }

  return result;
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
