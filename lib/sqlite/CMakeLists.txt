find_package(SQLite3 REQUIRED)

add_library(podrm-sqlite STATIC)
target_sources(podrm-sqlite PRIVATE connection.cpp cursor.cpp entry.cpp
                                    result.cpp row.cpp)
target_link_libraries(
  podrm-sqlite
  PUBLIC podrm-reflection
  PRIVATE SQLite::SQLite3 fmt::fmt)
