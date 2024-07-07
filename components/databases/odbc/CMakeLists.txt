find_package(ODBC REQUIRED)
find_package(fmt REQUIRED)

add_library(podrm-odbc STATIC)
target_sources(
  podrm-odbc
  PRIVATE lib/environment.cpp
          lib/error.cpp
          lib/string.cpp
          lib/connection.cpp
          lib/cursor.cpp
          lib/entry.cpp
          lib/result.cpp
          lib/row.cpp)
target_link_libraries(
  podrm-odbc
  PUBLIC podrm-metadata
  PRIVATE podrm-multilambda ODBC::ODBC fmt::fmt)
target_include_directories(podrm-odbc PUBLIC include)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
