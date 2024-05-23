#include "field.hpp"

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>

namespace {

struct Person {
  std::uint64_t id;

  std::string name;
};

struct NotPerson {};

} // namespace

template <>
constexpr auto podrm::EntityRegistration<Person> =
    podrm::EntityRegistrationData<Person>{
        .id = test::Field<Person, &Person::id>,
        .idMode = IdMode::Auto,
    };

namespace {

static_assert(podrm::DatabaseEntity<Person>);
static_assert(not podrm::DatabaseEntity<NotPerson>);

constexpr podrm::EntityDescription PersonDescription =
    podrm::DatabaseEntityDescription<Person>;

constexpr std::array ExpectedFields = {
    podrm::FieldDescription{
        .name = "id",
        .field =
            podrm::PrimitiveFieldDescription{
                .nativeType = podrm::NativeType::BigInt,
            },
    },
    podrm::FieldDescription{
        .name = "name",
        .field =
            podrm::PrimitiveFieldDescription{
                .nativeType = podrm::NativeType::String,
            },
    },
};

constexpr podrm::EntityDescription ExpectedDescription = {
    .name = "Person",
    .fields = ExpectedFields,
    .primaryKey = 0,
};

static_assert(PersonDescription == ExpectedDescription);

static_assert(PersonDescription.name == "Person");
static_assert(PersonDescription.fields[0].name == "id");
static_assert(std::get<podrm::PrimitiveFieldDescription>(
                  PersonDescription.fields[0].field) ==
              podrm::PrimitiveFieldDescription{
                  .nativeType = podrm::NativeType::BigInt});
static_assert(PersonDescription.fields[1].name == "name");
static_assert(std::get<podrm::PrimitiveFieldDescription>(
                  PersonDescription.fields[1].field) ==
              podrm::PrimitiveFieldDescription{
                  .nativeType = podrm::NativeType::String});

} // namespace
