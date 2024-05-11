#include <pfr-orm/definitions.hpp>
#include <pfr-orm/operations.hpp>
#include <pfr-orm/postges-helpers.hpp>

#include <cassert>

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
        .id = PFRORM_FIELD(Person, id),
        .idMode = IdMode::Auto,
    };

static_assert(pfrorm::DatabaseEntity<Person>);

int main(int argc, char **argv) {
  fmt::println("{}", pfrorm::DatabaseEntityDescription<Person>.name);

  pg_orm::Connection conn = pg_orm::connect(argv[1]);

  pg_orm::createTable<Person>(*conn);

  assert(!pg_orm::exists<Person>(*conn));
}
