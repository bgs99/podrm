cmake_minimum_required(VERSION 3.7)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(podrm LANGUAGES C CXX)

add_subdirectory(vendor)

find_package(fmt REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(podrm STATIC)
target_compile_features(podrm PUBLIC cxx_std_20)
target_sources(podrm PRIVATE lib/sqlite/utils.cpp lib/sqlite/operations.cpp
                             lib/postgres/utils.cpp lib/postgres/operations.cpp)
target_include_directories(podrm SYSTEM PUBLIC include)
target_link_libraries(
  podrm
  PUBLIC Boost::pfr
  PRIVATE PostgreSQL::PostgreSQL SQLite::SQLite3 fmt::fmt)

option(PFR_ORM_USE_GSL_SPAN
       "Use Microsoft.GSL for span implementation instead of std::span" OFF)
if(PFR_ORM_USE_GSL_SPAN)
  target_compile_definitions(podrm PUBLIC PFR_ORM_USE_GSL_SPAN)
  find_package(Microsoft.GSL REQUIRED)
  target_link_libraries(podrm PUBLIC Microsoft.GSL::GSL)
endif()

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  option(PFR_ORM_ASAN "Build podrm with address sanitizer" OFF)

  if(PFR_ORM_ASAN)
    target_compile_options(podrm PRIVATE -fsanitize=address)
    target_link_options(podrm PRIVATE -fsanitize=address)
  endif()

  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
