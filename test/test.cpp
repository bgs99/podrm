#include "field.hpp"

#include <podrm/api.hpp>
#include <podrm/definitions.hpp>
#include <podrm/detail/span.hpp>
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

struct Person {
  std::int64_t id;

  std::string name;
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
  podrm::detail::span<const char *const> args{
      argv,
      static_cast<std::size_t>(argc),
  };

  try {
    orm::Connection conn = args.size() <= 1 ? orm::Connection::inMemory("test")
                                            : orm::Connection::inFile(args[1]);

    orm::createTable<Person>(conn);

    assert(!orm::exists<Person>(conn));

    {
      std::optional<Person> person = orm::find<Person>(conn, 42);
      assert(!person.has_value());
    }

    {
      Person person{
          .id = 42,
          .name = "Alex",
      };

      orm::persist(conn, person);
    }

    assert(orm::exists<Person>(conn));

    {
      std::optional<Person> person = orm::find<Person>(conn, 42);
      assert(person.has_value());
      assert(person->id == 42);
      assert(person->name == "Alex");
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
