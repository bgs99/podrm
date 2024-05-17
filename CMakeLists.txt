cmake_minimum_required(VERSION 3.7)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pfr-orm LANGUAGES C CXX)

add_subdirectory(vendor)

find_package(fmt REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(pfr-orm STATIC)
target_compile_features(pfr-orm PUBLIC cxx_std_20)
target_sources(
  pfr-orm PRIVATE lib/sqlite/utils.cpp lib/sqlite/operations.cpp
                  lib/postgres/utils.cpp lib/postgres/operations.cpp)
target_include_directories(pfr-orm PUBLIC include)
target_link_libraries(
  pfr-orm
  PUBLIC Boost::pfr
  PRIVATE PostgreSQL::PostgreSQL SQLite::SQLite3 fmt::fmt)

option(PFR_ORM_USE_GSL_SPAN
       "Use Microsoft.GSL for span implementation instead of std::span" OFF)
if(PFR_ORM_USE_GSL_SPAN)
  target_compile_definitions(pfr-orm PUBLIC PFR_ORM_USE_GSL_SPAN)
  find_package(Microsoft.GSL REQUIRED)
  target_link_libraries(pfr-orm PUBLIC Microsoft.GSL::GSL)
endif()

option(PFR_ORM_ASAN "Build pfr-orm with address sanitizer" OFF)

if(PFR_ORM_ASAN)
  target_compile_options(pfr-orm PRIVATE -fsanitize=address)
  target_link_options(pfr-orm PRIVATE -fsanitize=address)
endif()

option(PFR_ORM_TEST "Generate test target for pfr-orm" OFF)

if(PFR_ORM_TEST)
  add_subdirectory(test)
endif()
