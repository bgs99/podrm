#include <pfr-orm/api.hpp>
#include <pfr-orm/definitions.hpp>

#include <cstdint>
#include <string>

struct Person {
  std::uint64_t id;

  std::string name;
};

template <> struct pfrorm::EntityRegistration<Person> {
  /// Identifier mode
  constexpr static IdMode Id = IdMode::Auto;

  /// Identifier field
  constexpr static std::uint64_t Person::*IdField = &Person::id;
};

static_assert(pfrorm::DatabaseEntity<Person>);
