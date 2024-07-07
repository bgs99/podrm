#include "comparisons.hpp"
#include "field.hpp"

#include <podrm/reflection/api.hpp>
#include <podrm/reflection/reflection.hpp>

#include <array>
#include <cstdint>
#include <string>

namespace {

struct Person {
  std::int64_t id;

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

static_assert(podrm::RegisteredEntity<Person>);
static_assert(not podrm::RegisteredEntity<NotPerson>);

constexpr podrm::EntityDescription PersonDescription =
    podrm::DatabaseEntityDescription<Person>.value();

constexpr std::array ExpectedFields = {
    podrm::FieldDescription{
        .name = "id",
        .field =
            podrm::PrimitiveFieldDescription{
                .imageType = podrm::ImageType::Int,
            },
    },
    podrm::FieldDescription{
        .name = "name",
        .field =
            podrm::PrimitiveFieldDescription{
                .imageType = podrm::ImageType::String,
            },
    },
};

constexpr podrm::EntityDescription ExpectedDescription = {
    .name = "Person",
    .fields = ExpectedFields,
    .primaryKey = 0,
};

static_assert(PersonDescription == ExpectedDescription);

} // namespace
