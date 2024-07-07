#pragma once

#include <podrm/metadata.hpp>
#include <podrm/reflection/api.hpp>
#include <podrm/reflection/detail/concepts.hpp>
#include <podrm/reflection/detail/pfr.hpp>
#include <podrm/reflection/detail/type_name.hpp>
#include <podrm/reflection/primary_key.hpp>
#include <podrm/reflection/relations.hpp>
#include <podrm/span.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/pfr/core.hpp>
#include <boost/pfr/core_name.hpp>
#include <boost/pfr/tuple_size.hpp>

namespace podrm {

template <typename T> constexpr ImageType getImageType() {
  if constexpr (std::is_same_v<T, std::int64_t>) {
    return ImageType::Int;
  }
  if constexpr (std::is_same_v<T, std::uint64_t>) {
    return ImageType::Uint;
  }
  if constexpr (std::is_same_v<T, double>) {
    return ImageType::Float;
  }
  if constexpr (std::is_same_v<T, std::string> ||
                std::is_same_v<T, std::string_view>) {
    return ImageType::String;
  }
  if constexpr (std::is_same_v<T, bool>) {
    return ImageType::Bool;
  }
  if constexpr (std::is_same_v<T, span<const std::byte>> ||
                std::is_same_v<T, std::vector<std::byte>>) {
    return ImageType::Bytes;
  }

  throw std::runtime_error{"Unsupported image type"};
}

template <ImageType Type> constexpr auto castImage(const FromImage image) {
  if constexpr (Type == ImageType::Int) {
    return std::get<std::int64_t>(image);
  }
  if constexpr (Type == ImageType::Uint) {
    return std::get<std::uint64_t>(image);
  }
  if constexpr (Type == ImageType::Float) {
    return std::get<double>(image);
  }
  if constexpr (Type == ImageType::String) {
    return std::get<std::string_view>(image);
  }
  if constexpr (Type == ImageType::Bool) {
    return std::get<bool>(image);
  }
  if constexpr (Type == ImageType::Bytes) {
    return std::get<span<const std::byte>>(image);
  }

  throw std::runtime_error{"Unsupported image type"};
}

namespace detail {

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
getFieldDescriptions();

template <detail::Reflectable T>
constexpr std::array<FieldDescription, boost::pfr::tuple_size_v<T>>
    FieldDescriptions = getFieldDescriptions<T>();

template <RegisteredPrimitive Field>
constexpr ImageType PrimitiveImageType = getImageType<std::invoke_result_t<
    decltype(ValueRegistration<Field>::asImage), const Field &>>();

template <typename Field>
constexpr std::optional<ForeignKeyDescription> ForeignKeyConstraint =
    std::nullopt;

template <typename Entity, typename KeyType>
constexpr std::optional<ForeignKeyDescription>
    ForeignKeyConstraint<ForeignKey<Entity, KeyType>> = ForeignKeyDescription{
        .entity = SimpleTypeName<Entity>,
        .field = PrimaryKeyName<Entity>,
    };

template <RegisteredPrimitive Field>
constexpr PrimitiveFieldDescription PrimitiveDescription{
    .imageType = PrimitiveImageType<Field>,
    .asImage = [](const void *value) -> AsImage {
      return ValueRegistration<Field>::asImage(
          *static_cast<const Field *>(value));
    },
    .fromImage =
        [](const FromImage image, void *field) {
          *static_cast<Field *>(field) = ValueRegistration<Field>::fromImage(
              castImage<PrimitiveImageType<Field>>(image));
        },
    .foreignKeyContraint = ForeignKeyConstraint<Field>,
};

template <RegisteredPrimitive Field>
constexpr FieldDescription
getFieldDescriptionOfField(const std::string_view name,
                           const MemberPtrFn memberPtr,
                           const ConstMemberPtrFn constMemberPtr) {
  return FieldDescription{
      .name{name},
      .memberPtr = memberPtr,
      .constMemberPtr = constMemberPtr,
      .field = PrimitiveDescription<Field>,
  };
}

template <RegisteredComposite Field>
  requires(not RegisteredPrimitive<Field>)
constexpr FieldDescription
getFieldDescriptionOfField(const std::string_view name,
                           const MemberPtrFn memberPtr,
                           const ConstMemberPtrFn constMemberPtr) {
  return FieldDescription{
      .name{name},
      .memberPtr = memberPtr,
      .constMemberPtr = constMemberPtr,
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
      names[Idx],
      [](void *data) -> void * {
        T &instance = *static_cast<T *>(data);
        return &boost::pfr::get<Idx>(instance);
      },
      [](const void *data) -> const void * {
        const T &instance = *static_cast<const T *>(data);
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

template <RegisteredEntity T>
constexpr std::optional<EntityDescription> DatabaseEntityDescription<T> =
    EntityDescription{
        .idMode = EntityRegistration<T>.idMode,
        .name = detail::SimpleTypeName<T>,
        .fields = detail::FieldDescriptions<T>,
        .primaryKey = EntityRegistration<T>.id.get(),
    };

} // namespace podrm
