#include "field.hpp"

#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>
#include <pfr-orm/postgres/operations.hpp>
#include <pfr-orm/postgres/utils.hpp>

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <span>
#include <string>

#include <fmt/core.h>

namespace pg_orm = pfrorm::postgres;

namespace {

struct Person {
  std::uint64_t id;

  std::string name;
};

} // namespace

template <>
constexpr auto pfrorm::EntityRegistration<Person> =
    pfrorm::EntityRegistrationData<Person>{
        .id = test::Field<Person, &Person::id>,
        .idMode = IdMode::Auto,
    };

static_assert(pfrorm::DatabaseEntity<Person>);

int main(int argc, char **argv) {
  const std::span<char *> args{argv, static_cast<std::size_t>(argc)};

  try {
    pg_orm::Connection conn{args[1]};

    pg_orm::createTable<Person>(conn);

    assert(!pg_orm::exists<Person>(conn));

    return 0;
  } catch (const std::exception &ex) {
    fmt::print("{}\n", ex.what());
    return 1;
  } catch (...) {
    fmt::print("Unknown exception\n");
    return 1;
  }
}
