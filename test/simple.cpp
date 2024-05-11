#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>

#include <array>
#include <cstdint>
#include <string>

namespace {

struct Person {
  std::uint64_t id;

  std::string name;
};

struct NotPerson {};

} // namespace

template <>
constexpr auto pfrorm::EntityRegistration<Person> =
    pfrorm::EntityRegistrationData<Person>{
        .id = PFRORM_FIELD(Person, id),
        .idMode = IdMode::Auto,
    };

namespace {

static_assert(pfrorm::DatabaseEntity<Person>);
static_assert(not pfrorm::DatabaseEntity<NotPerson>);

constexpr pfrorm::EntityDescription PersonDescription =
    pfrorm::DatabaseEntityDescription<Person>;

constexpr std::array ExpectedFields = {
    pfrorm::FieldDescription{
        .name = "id",
        .field =
            pfrorm::PrimitiveFieldDescription{
                .nativeType = pfrorm::NativeType::BigInt,
            },
    },
    pfrorm::FieldDescription{
        .name = "name",
        .field =
            pfrorm::PrimitiveFieldDescription{
                .nativeType = pfrorm::NativeType::String,
            },
    },
};

constexpr pfrorm::EntityDescription ExpectedDescription = {
    .name = "Person",
    .fields = ExpectedFields,
    .primaryKey = 0,
};

static_assert(PersonDescription == ExpectedDescription);

static_assert(PersonDescription.name == "Person");
static_assert(PersonDescription.fields[0].name == "id");
static_assert(std::get<pfrorm::PrimitiveFieldDescription>(
                  PersonDescription.fields[0].field) ==
              pfrorm::PrimitiveFieldDescription{
                  .nativeType = pfrorm::NativeType::BigInt});
static_assert(PersonDescription.fields[1].name == "name");
static_assert(std::get<pfrorm::PrimitiveFieldDescription>(
                  PersonDescription.fields[1].field) ==
              pfrorm::PrimitiveFieldDescription{
                  .nativeType = pfrorm::NativeType::String});

} // namespace
