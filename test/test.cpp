#include "Person.hpp"
#include <pfr-orm/operations.hpp>
#include <pfr-orm/postges-helpers.hpp>

#include <cassert>

namespace pg_orm = pfrorm::postgres;

int main(int argc, char **argv) {
  pg_orm::Connection conn = pg_orm::connect(argv[1]);

  pg_orm::createTable<Person>(*conn);

  assert(!pg_orm::exists<Person>(*conn));
}
