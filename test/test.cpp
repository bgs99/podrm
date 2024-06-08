#include "field.hpp"

#include <podrm/api.hpp>
#include <podrm/reflection.hpp>
#include <podrm/relations.hpp>
#include <podrm/span.hpp>
#include <podrm/sqlite/operations.hpp>
#include <podrm/sqlite/utils.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <string>

#include <fmt/core.h>

namespace orm = podrm::sqlite;

namespace {

struct Address {
  std::int64_t id;

  std::string postalCode;
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
};

} // namespace

template <>
constexpr auto podrm::EntityRegistration<Person> =
    podrm::EntityRegistrationData<Person>{
        .id = test::Field<Person, &Person::id>,
        .idMode = IdMode::Auto,
    };

static_assert(podrm::DatabaseEntity<Person>);

int main(const int argc, const char **argv) {
  podrm::span<const char *const> args{
      argv,
      static_cast<std::size_t>(argc),
  };

  try {
    orm::Connection conn = args.size() <= 1 ? orm::Connection::inMemory("test")
                                            : orm::Connection::inFile(args[1]);

    orm::createTable<Address>(conn);
    orm::createTable<Person>(conn);

    assert(!orm::exists<Person>(conn));

    {
      std::optional<Person> person = orm::find<Person>(conn, 42);
      assert(!person.has_value());
    }

    {
      Address address{
          .id = 3,
          .postalCode = "abc",
      };

      orm::persist(conn, address);

      Person person{
          .id = 42,
          .name = "Alex",
          .address{.key = address.id},
      };

      orm::persist(conn, person);
    }

    assert(orm::exists<Person>(conn));

    {
      const std::optional<Person> person = orm::find<Person>(conn, 42);
      assert(person.has_value());
      assert(person->id == 42);
      assert(person->name == "Alex");
      assert(person->address.key == 3);

      const std::optional<Address> address =
          orm::find<Address>(conn, person->address.key);
      assert(address.has_value());
      assert(address->id == 3);
      assert(address->postalCode == "abc");
    }

    {
      const Person person{
          .id = 42,
          .name = "Anne",
          .address{.key = 3},
      };

      orm::update(conn, person);
    }

    {
      std::optional<Person> person = orm::find<Person>(conn, 42);
      assert(person.has_value());
      assert(person->id == 42);
      assert(person->name == "Anne");
    }

    orm::erase<Person>(conn, 42);

    assert(!orm::exists<Person>(conn));

    return 0;
  } catch (const std::exception &ex) {
    fmt::print("{}\n", ex.what());
    return 1;
  } catch (...) {
    fmt::print("Unknown exception\n");
    return 1;
  }
}
