#pragma once

#include <podrm/api.hpp>
#include <podrm/detail/pfr.hpp>
#include <podrm/detail/span.hpp>
#include <podrm/detail/type_name.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <boost/pfr/core.hpp>
#include <boost/pfr/core_name.hpp>
#include <boost/pfr/tuple_size.hpp>

namespace podrm {

struct PrimitiveFieldDescription {
  NativeType nativeType;
};

struct FieldDescription;

struct CompositeFieldDescription {
  detail::span<const FieldDescription> fields;
};

using MemberPtrFn = void *(*)(void *);

struct FieldDescription {
  std::string_view name;

  MemberPtrFn memberPtr;
  std::variant<PrimitiveFieldDescription, CompositeFieldDescription> field;
};

struct EntityDescription {
  IdMode idMode;
  std::string_view name;
  detail::span<const FieldDescription> fields;
  std::size_t primaryKey;
};

template <typename T>
concept DatabaseEntity =
    detail::Reflectable<T> && std::is_same_v<decltype(EntityRegistration<T>),
                                             const EntityRegistrationData<T>>;

template <typename T>
concept DatabaseComposite = detail::Reflectable<T> &&
                            std::is_same_v<decltype(CompositeRegistration<T>),
                                           const CompositeRegistrationData<T>>;

template <typename T>
concept DatabasePrimitive = std::is_same_v<decltype(ValueRegistration<T>),
                                           const ValueRegistrationData<T>>;

namespace detail {

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
getFieldDescriptions();

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
    FieldDescriptions = getFieldDescriptions<T>();

template <DatabasePrimitive Field>
constexpr FieldDescription
getFieldDescriptionOfField(const std::string_view name,
                           const MemberPtrFn memberPtr) {
  return FieldDescription{
      .name{name},
      .memberPtr = memberPtr,
      .field =
          PrimitiveFieldDescription{
              .nativeType = ValueRegistration<Field>.nativeType,
          },
  };
}

template <DatabaseComposite Field>
  requires(not DatabasePrimitive<Field>)
constexpr FieldDescription
getFieldDescriptionOfField(const std::string_view name,
                           const MemberPtrFn memberPtr) {
  return FieldDescription{
      .name{name},
      .memberPtr = memberPtr,
      .field =
          CompositeFieldDescription{
              .fields = FieldDescriptions<Field>,
          },
  };
}

template <detail::Reflectable T, std::size_t FieldsCount, std::size_t... Idx>
constexpr std::array<FieldDescription, FieldsCount>
getFieldDescriptions(std::array<std::string_view, FieldsCount> names,
                     std::index_sequence<Idx...> /*indices*/) {
  return {getFieldDescriptionOfField<boost::pfr::tuple_element_t<Idx, T>>(
      names[Idx], [](void *data) -> void * {
        T &instance = *static_cast<T *>(data);
        return &boost::pfr::get<Idx>(instance);
      })...};
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
    .idMode = EntityRegistration<T>.idMode,
    .name = detail::SimpleTypeName<T>,
    .fields = detail::FieldDescriptions<T>,
    .primaryKey = EntityRegistration<T>.id.get(),
};

template <DatabaseEntity T>
using PrimaryKeyType =
    boost::pfr::tuple_element_t<DatabaseEntityDescription<T>.primaryKey, T>;

} // namespace podrm
