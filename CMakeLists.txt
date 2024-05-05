cmake_minimum_required(VERSION 3.7)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(pfr-orm LANGUAGES C CXX)

add_subdirectory(vendor)

find_package(fmt REQUIRED)
find_package(PostgreSQL REQUIRED)

add_library(pfr-orm STATIC)
target_compile_features(pfr-orm PUBLIC cxx_std_20)
target_sources(pfr-orm PRIVATE lib/detail/escaping.cpp
                               lib/detail/operations.cpp)
target_include_directories(pfr-orm PUBLIC include)
target_link_libraries(
  pfr-orm
  PUBLIC Boost::pfr PostgreSQL::PostgreSQL
  PRIVATE fmt::fmt)

option(PFR_ORM_ASAN "Build pfr-orm with address sanitizer" OFF)

if(PFR_ORM_ASAN)
  target_compile_options(pfr-orm PRIVATE -fsanitize=address)
  target_link_options(pfr-orm PRIVATE -fsanitize=address)
endif()

option(PFR_ORM_TEST "Generate test target for pfr-orm" OFF)

if(PFR_ORM_TEST)
  add_subdirectory(test)
endif()
