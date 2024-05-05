#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>

#include <cstdint>
#include <string>

namespace {

struct Person {
  std::uint64_t id;

  std::string name;
};

} // namespace

template <> struct pfrorm::EntityRegistration<Person> {
  /// Identifier mode
  constexpr static IdMode Id = IdMode::Auto;

  /// Identifier field
  constexpr static std::uint64_t Person::*IdField = &Person::id;
};

namespace {

static_assert(pfrorm::DatabaseEntity<Person>);

constexpr pfrorm::EntityDescription PersonDescription =
    pfrorm::DatabaseEntityDescription<Person>;

static_assert(PersonDescription.name == "Person");
static_assert(PersonDescription.fields[0].name == "id");
static_assert(std::get<pfrorm::PrimitiveFieldDescription>(
                  PersonDescription.fields[0].field) ==
              pfrorm::PrimitiveFieldDescription{.nativeType = "BIGINT"});
static_assert(PersonDescription.fields[1].name == "name");
static_assert(std::get<pfrorm::PrimitiveFieldDescription>(
                  PersonDescription.fields[1].field) ==
              pfrorm::PrimitiveFieldDescription{.nativeType = "VARCHAR"});

} // namespace
