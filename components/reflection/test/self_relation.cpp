#include "comparisons.hpp"
#include "field.hpp"

#include <podrm/reflection/api.hpp>
#include <podrm/reflection/reflection.hpp>
#include <podrm/reflection/relations.hpp>

#include <array>
#include <cstdint>
#include <string>

namespace {

struct Person {
  std::int64_t id;

  std::string name;

  podrm::ForeignKey<Person, std::int64_t> parent;
};

} // namespace

template <>
constexpr auto podrm::EntityRegistration<Person> =
    podrm::EntityRegistrationData<Person>{
        .id = test::Field<Person, &Person::id>,
        .idMode = IdMode::Auto,
    };

namespace {

static_assert(podrm::RegisteredEntity<Person>);

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
    podrm::FieldDescription{
        .name = "parent",
        .field =
            podrm::PrimitiveFieldDescription{
                .imageType = podrm::ImageType::Int,
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
