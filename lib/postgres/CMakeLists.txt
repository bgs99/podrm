find_package(PostgreSQL REQUIRED)

add_library(podrm-postgres STATIC)
target_sources(podrm-postgres PRIVATE connection.cpp result.cpp str.cpp)
target_link_libraries(
  podrm-postgres
  PUBLIC podrm-reflection
  PRIVATE PostgreSQL::PostgreSQL fmt::fmt)
