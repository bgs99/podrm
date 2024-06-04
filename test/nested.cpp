#include "comparisons.hpp"
#include "field.hpp"

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>

#include <array>
#include <cstdint>

namespace {

struct Composite {
  std::int64_t a;
  std::int64_t b;
};

struct Entity {
  std::int64_t id;

  Composite composite;
};

} // namespace

template <>
constexpr auto podrm::CompositeRegistration<Composite> =
    CompositeRegistrationData<Composite>{};

template <>
constexpr auto podrm::EntityRegistration<Entity> =
    podrm::EntityRegistrationData<Entity>{
        .id = test::Field<Entity, &Entity::id>,
        .idMode = IdMode::Auto,
    };

namespace {

static_assert(podrm::DatabaseComposite<Composite>);
static_assert(podrm::DatabaseEntity<Entity>);
static_assert(not podrm::DatabaseEntity<Composite>);
static_assert(not podrm::DatabaseComposite<Entity>);

constexpr podrm::EntityDescription EntityDescription =
    podrm::DatabaseEntityDescription<Entity>;

constexpr std::array ExpectedCompositeFields = {
    podrm::FieldDescription{
        .name = "a",
        .field =
            podrm::PrimitiveFieldDescription{
                .imageType = podrm::ImageType::Int,
            },
    },
    podrm::FieldDescription{
        .name = "b",
        .field =
            podrm::PrimitiveFieldDescription{
                .imageType = podrm::ImageType::Int,
            },
    },
};

constexpr podrm::CompositeFieldDescription ExpectedCompositeDescription = {
    .fields = ExpectedCompositeFields,
};

constexpr std::array ExpectedEntityFields = {
    podrm::FieldDescription{
        .name = "id",
        .field =
            podrm::PrimitiveFieldDescription{
                .imageType = podrm::ImageType::Int,
            },
    },
    podrm::FieldDescription{
        .name = "composite",
        .field =
            podrm::CompositeFieldDescription{
                .fields = ExpectedCompositeFields,
            },
    },
};

constexpr podrm::EntityDescription ExpectedEntityDescription = {
    .name = "Entity",
    .fields = ExpectedEntityFields,
    .primaryKey = 0,
};

static_assert(EntityDescription == ExpectedEntityDescription);

} // namespace
