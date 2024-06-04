#pragma once

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>
#include <podrm/detail/fn_deduction.hpp>

namespace podrm {

template <DatabaseEntity Entity> struct ForeignKey {
  PrimaryKeyType<Entity> key;
};

template <DatabaseEntity Entity>
  requires DatabasePrimitive<PrimaryKeyType<Entity>>
struct ValueRegistration<ForeignKey<Entity>> {
  using KeyType = PrimaryKeyType<Entity>;
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
