#pragma once

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>
#include <podrm/detail/concepts.hpp>
#include <podrm/detail/fn_deduction.hpp>

namespace podrm {

template <typename Entity, typename KeyType = PrimaryKeyType<Entity>>
struct ForeignKey {
  KeyType key;
};

template <typename Entity, typename KeyType>
struct ValueRegistration<ForeignKey<Entity, KeyType>> {
  static_assert(DatabaseEntity<Entity>);
  static_assert(DatabasePrimitive<KeyType>);
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
