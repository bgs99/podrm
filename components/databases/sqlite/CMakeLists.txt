find_package(SQLite3 REQUIRED)
find_package(fmt REQUIRED)

add_library(podrm-sqlite STATIC)
target_sources(podrm-sqlite PRIVATE lib/connection.cpp lib/cursor.cpp
                                    lib/entry.cpp lib/result.cpp lib/row.cpp)
target_link_libraries(
  podrm-sqlite
  PUBLIC podrm::metadata
  PRIVATE podrm::multilambda SQLite::SQLite3 fmt::fmt)
target_include_directories(podrm-sqlite PUBLIC include)

add_library(podrm::sqlite ALIAS podrm-sqlite)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
