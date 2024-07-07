#pragma once

#include <podrm/reflection/api.hpp>
#include <podrm/reflection/detail/concepts.hpp>
#include <podrm/reflection/detail/fn_deduction.hpp>
#include <podrm/reflection/primary_key.hpp>

#include <utility>

namespace podrm {

template <typename Entity, typename KeyType = PrimaryKeyType<Entity>>
struct ForeignKey {
  KeyType key;

  friend constexpr bool
  operator==(const ForeignKey &, const ForeignKey &) noexcept(
      noexcept(std::declval<KeyType>() == std::declval<KeyType>())) = default;
};

template <typename Entity, typename KeyType>
struct ValueRegistration<ForeignKey<Entity, KeyType>> {
  static_assert(RegisteredEntity<Entity>);
  static_assert(RegisteredPrimitive<KeyType>);
  static_assert(detail::same_as<KeyType, PrimaryKeyType<Entity>>);

  using KeyRegistration = ValueRegistration<KeyType>;
  using AsImage = detail::UnaryFnReturnType<&KeyRegistration::asImage>;
  using FromImage = detail::UnaryFnArgumentType<&KeyRegistration::fromImage>;

  static AsImage asImage(const ForeignKey<Entity> &value) {
    return KeyRegistration::asImage(value.key);
  }

  static ForeignKey<Entity> fromImage(FromImage image) {
    return ForeignKey<Entity>{.key = KeyRegistration::fromImage(image)};
  }
};

} // namespace podrm
