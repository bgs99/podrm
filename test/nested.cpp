#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>

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

template <> struct pfrorm::CompositeRegistration<Composite> {};

template <> struct pfrorm::EntityRegistration<Entity> {
  /// Identifier mode
  constexpr static IdMode Id = IdMode::Auto;

  /// Identifier field
  constexpr static std::uint64_t Entity::*IdField = &Entity::id;
};

namespace {

static_assert(pfrorm::DatabaseComposite<Composite>);
static_assert(pfrorm::DatabaseEntity<Entity>);

constexpr pfrorm::EntityDescription EntityDescription =
    pfrorm::DatabaseEntityDescription<Entity>;

static_assert(EntityDescription.name == "Entity");
static_assert(EntityDescription.fields[0].name == "id");
static_assert(std::get<pfrorm::PrimitiveFieldDescription>(
                  EntityDescription.fields[0].field) ==
              pfrorm::PrimitiveFieldDescription{.nativeType = "BIGINT"});
static_assert(EntityDescription.fields[1].name == "composite");

constexpr std::array CompositeFieldsDescriptions = {
    pfrorm::FieldDescription{
        .name = "a",
        .field = pfrorm::PrimitiveFieldDescription{.nativeType = "BIGINT"}},
    pfrorm::FieldDescription{
        .name = "b",
        .field = pfrorm::PrimitiveFieldDescription{.nativeType = "BIGINT"}},
};

constexpr pfrorm::CompositeFieldDescription CompositeDescription = {
    .fields = CompositeFieldsDescriptions,
};

static_assert(std::get<pfrorm::CompositeFieldDescription>(
                  EntityDescription.fields[1].field) == CompositeDescription);

} // namespace
