find_package(PostgreSQL REQUIRED)
find_package(fmt REQUIRED)

add_library(podrm-postgres STATIC)
target_sources(podrm-postgres PRIVATE lib/connection.cpp lib/result.cpp
                                      lib/str.cpp)
target_link_libraries(
  podrm-postgres
  PUBLIC podrm-metadata
  PRIVATE podrm-multilambda PostgreSQL::PostgreSQL fmt::fmt)
target_include_directories(podrm-postgres PUBLIC include)
