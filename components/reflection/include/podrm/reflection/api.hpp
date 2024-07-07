#pragma once

#include <podrm/metadata.hpp>
#include <podrm/reflection/detail/concepts.hpp>
#include <podrm/reflection/detail/field.hpp>
#include <podrm/reflection/detail/pfr.hpp>
#include <podrm/span.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include <boost/pfr.hpp>
#include <boost/pfr/config.hpp>

static_assert(BOOST_PFR_ENABLED, "Boost.PFR is not supported, cannot build");
static_assert(BOOST_PFR_CORE_NAME_ENABLED,
              "Boost.PFR does not support field name extraction, cannot build");

namespace podrm {

template <typename T> class FieldDescriptor {
public:
  [[nodiscard]] constexpr std::size_t get() const { return this->field; }

  template <const auto T::*MemberPtr>
  constexpr static FieldDescriptor fromMember() {
    return FieldDescriptor{detail::getFieldIndex<T, MemberPtr>()};
  }

private:
  std::size_t field;

  constexpr explicit FieldDescriptor(const std::size_t field) : field(field) {}
};

template <typename T, const auto MemberPtr>
  requires(std::is_member_pointer_v<decltype(MemberPtr)>)
constexpr FieldDescriptor<T> FieldOf =
    FieldDescriptor<T>::template fromMember<MemberPtr>();

template <const auto MemberPtr>
  requires(std::is_member_pointer_v<decltype(MemberPtr)>)
constexpr FieldDescriptor<detail::MemberPtrClass<MemberPtr>> Field =
    FieldDescriptor<detail::MemberPtrClass<MemberPtr>>::template fromMember<
        MemberPtr>();

template <typename T> struct EntityRegistrationData {
  FieldDescriptor<T> id;
  IdMode idMode;
};

/// ORM entity registration
/// @tparam T registered type
template <typename T>
constexpr std::optional<EntityRegistrationData<T>> EntityRegistration =
    std::nullopt;

template <typename T> struct CompositeRegistrationData {};

/// ORM composite value registration
/// @tparam T registered type
template <typename T>
constexpr std::optional<CompositeRegistrationData<T>> CompositeRegistration =
    std::nullopt;

using AsImage = std::variant<span<const std::byte>, double,
                             std::vector<std::byte>, std::string_view,
                             std::string, std::uint64_t, std::int64_t, bool>;

using FromImage = std::variant<span<const std::byte>, double, std::string_view,
                               std::uint64_t, std::int64_t, bool>;

template <typename T> struct ValueRegistration;

template <> struct ValueRegistration<int64_t> {
  static std::int64_t asImage(const std::int64_t value) { return value; }
  static std::int64_t fromImage(const std::int64_t image) { return image; }
};

template <> struct ValueRegistration<uint64_t> {
  static std::uint64_t asImage(const std::uint64_t value) { return value; }
  static std::uint64_t fromImage(const std::uint64_t image) { return image; }
};

template <> struct ValueRegistration<std::string> {
  static std::string_view asImage(const std::string &value) { return value; }
  static std::string fromImage(const std::string_view image) {
    return std::string{image};
  }
};

template <> struct ValueRegistration<std::string_view> {
  static std::string_view asImage(const std::string_view &value) {
    return value;
  }
  static std::string_view fromImage(const std::string_view image) {
    return image;
  }
};

template <typename T>
concept RegisteredEntity =
    detail::Reflectable<T> && std::is_same_v<decltype(EntityRegistration<T>),
                                             const EntityRegistrationData<T>>;

template <typename T>
concept RegisteredComposite =
    detail::Reflectable<T> &&
    std::is_same_v<decltype(CompositeRegistration<T>),
                   const CompositeRegistrationData<T>>;

template <typename T>
concept RegisteredPrimitive = requires(const T &value) {
  { ValueRegistration<T>::asImage(value) } -> detail::convertible_to<AsImage>;
  {
    ValueRegistration<T>::fromImage(ValueRegistration<T>::asImage(value))
  } -> detail::same_as<T>;
};

/// Tag to be used with boost::pfr::is_reflectable*
struct ReflectionTag;

} // namespace podrm
