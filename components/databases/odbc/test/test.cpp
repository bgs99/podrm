#include "field.hpp"

#include <podrm/metadata.hpp>
#include <podrm/odbc.hpp>
#include <podrm/reflection.hpp>
#include <podrm/span.hpp>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <optional>
#include <ostream>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <fmt/core.h>
#include <fmt/ostream.h>

namespace orm = podrm::odbc;

namespace {

struct Address {
  std::int64_t id;

  std::string postalCode;

  friend constexpr bool operator==(const Address &,
                                   const Address &) noexcept = default;

  friend std::ostream &operator<<(std::ostream &os, const Address &address) {
    fmt::print(os, "{{id: {}, postalCode: \"{}\"}}", address.id,
               address.postalCode);
    return os;
  }
};

} // namespace

template <>
constexpr auto podrm::EntityRegistration<Address> =
    podrm::EntityRegistrationData<Address>{
        .id = test::Field<Address, &Address::id>,
        .idMode = IdMode::Auto,
    };

namespace {

struct Person {
  std::int64_t id;

  std::string name;

  podrm::ForeignKey<Address> address;

  friend constexpr bool operator==(const Person &,
                                   const Person &) noexcept = default;

  friend std::ostream &operator<<(std::ostream &os, const Person &person) {
    fmt::print(os, "{{id: {}, name: \"{}\", address: {}}}", person.id,
               person.name, person.address.key);
    return os;
  }
};

} // namespace

template <>
constexpr auto podrm::EntityRegistration<Person> =
    podrm::EntityRegistrationData<Person>{
        .id = test::Field<Person, &Person::id>,
        .idMode = IdMode::Auto,
    };

static_assert(podrm::DatabaseEntity<Person>);

TEST_CASE("ODBC works", "[odbc]") {
  orm::Environment env;

  orm::Database db = orm::Database::fromConnectionString(
      env, std::getenv("PODRM_ODBC_CONNECTION_STRING"));

  REQUIRE_NOTHROW(db.createTable<Address>());
  REQUIRE_NOTHROW(db.createTable<Person>());

  REQUIRE_FALSE(db.exists<Address>());
  REQUIRE_FALSE(db.exists<Person>());

  SECTION("Foreign key constraints are enforced") {
    Person person{
        .id = 0,
        .name = "Alex",
        .address{.key = 42},
    };

    CHECK_THROWS(db.persist(person));
  }

  Address address{
      .id = 0,
      .postalCode = "abc",
  };

  REQUIRE_NOTHROW(db.persist(address));

  Person person{
      .id = 0,
      .name = "Alex",
      .address{.key = address.id},
  };

  REQUIRE_NOTHROW(db.persist(person));

  SECTION("find on non-existent id returns nullopt") {
    const std::optional<Person> person = db.find<Person>(42);
    CHECK_FALSE(person.has_value());
  }

  SECTION("erase on non-existent id throws") {
    CHECK_THROWS(db.erase<Person>(42));
  }

  SECTION("update on non-existent id throws") {
    Person newPerson{
        .id = 42,
        .name = "Anne",
        .address{.key = address.id},
    };

    CHECK_THROWS(db.update<Person>(newPerson));
  }

  SECTION("erase of referenced entity throws") {
    REQUIRE_THROWS(db.erase<Address>(address.id));
  }

  SECTION("find on existing id returns existing value") {
    const std::optional<Person> personFound = db.find<Person>(person.id);
    REQUIRE(personFound.has_value());
    CHECK(person == *personFound);
  }

  SECTION("erase on existing id erases existing value") {
    REQUIRE_NOTHROW(db.erase<Person>(person.id));

    const std::optional<Person> personFound = db.find<Person>(person.id);
    CHECK_FALSE(personFound.has_value());
  }

  SECTION("update on existing id updates existing value") {
    person.name = "Anne";
    REQUIRE_NOTHROW(db.update(person));

    const std::optional<Person> personFound = db.find<Person>(person.id);
    REQUIRE(personFound.has_value());
    CHECK(personFound->name == "Anne");
  }

  SECTION("iterate iterates over existing entities") {
    Person newPerson{
        .id = 1,
        .name = "John",
        .address{.key = address.id},
    };

    REQUIRE_NOTHROW(db.persist(newPerson));

    int i = 0;
    std::array<std::reference_wrapper<const Person>, 2> expected = {
        person,
        newPerson,
    };

    for (const Person &result : db.iterate<Person>()) {
      CHECK(result == expected.at(i).get());
      ++i;
    }

    CHECK(i == 2);
  }
}
