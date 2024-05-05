#include "pfr-orm/definitions.hpp"
#include <iostream>
#include <pfr-orm/operations.hpp>
#include <pfr-orm/postges-helpers.hpp>

#include <cassert>

namespace pg_orm = pfrorm::postgres;

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

static_assert(pfrorm::DatabaseEntity<Person>);

int main(int argc, char **argv) {
  std::cout << pfrorm::DatabaseEntityDescription<Person>.name << std::endl;

  pg_orm::Connection conn = pg_orm::connect(argv[1]);

  pg_orm::createTable<Person>(*conn);

  assert(!pg_orm::exists<Person>(*conn));
}
