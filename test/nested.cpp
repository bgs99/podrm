#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>

#include <array>
#include <cstdint>

namespace {

struct Composite {
  std::uint64_t a;
  std::uint64_t b;
};

struct Entity {
  std::uint64_t id;

  Composite composite;
};

} // namespace

template <>
constexpr auto pfrorm::CompositeRegistration<Composite> =
    CompositeRegistrationData<Composite>{};

template <>
constexpr auto pfrorm::EntityRegistration<Entity> =
    pfrorm::EntityRegistrationData<Entity>{
        .id = Field<&Entity::id>,
        .idMode = IdMode::Auto,
    };

namespace {

static_assert(pfrorm::DatabaseComposite<Composite>);
static_assert(pfrorm::DatabaseEntity<Entity>);
static_assert(not pfrorm::DatabaseEntity<Composite>);
static_assert(not pfrorm::DatabaseComposite<Entity>);

constexpr pfrorm::EntityDescription EntityDescription =
    pfrorm::DatabaseEntityDescription<Entity>;

constexpr std::array ExpectedCompositeFields = {
    pfrorm::FieldDescription{
        .name = "a",
        .field =
            pfrorm::PrimitiveFieldDescription{
                .nativeType = pfrorm::NativeType::BigInt,
            },
    },
    pfrorm::FieldDescription{
        .name = "b",
        .field =
            pfrorm::PrimitiveFieldDescription{
                .nativeType = pfrorm::NativeType::BigInt,
            },
    },
};

constexpr pfrorm::CompositeFieldDescription ExpectedCompositeDescription = {
    .fields = ExpectedCompositeFields,
};

constexpr std::array ExpectedEntityFields = {
    pfrorm::FieldDescription{
        .name = "id",
        .field =
            pfrorm::PrimitiveFieldDescription{
                .nativeType = pfrorm::NativeType::BigInt,
            },
    },
    pfrorm::FieldDescription{
        .name = "composite",
        .field =
            pfrorm::CompositeFieldDescription{
                .fields = ExpectedCompositeFields,
            },
    },
};

constexpr pfrorm::EntityDescription ExpectedEntityDescription = {
    .name = "Entity",
    .fields = ExpectedEntityFields,
    .primaryKey = 0,
};

static_assert(EntityDescription == ExpectedEntityDescription);

} // namespace
