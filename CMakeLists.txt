cmake_minimum_required(VERSION 3.7)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(pfr-orm LANGUAGES C CXX)

add_subdirectory(vendor)

find_package(fmt REQUIRED)

add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)

find_package(PostgreSQL REQUIRED)

add_library(pfr-orm STATIC)
target_sources(pfr-orm PRIVATE lib/detail/escaping.cpp
                               lib/detail/operations.cpp)
target_include_directories(pfr-orm PUBLIC include)
target_link_libraries(
  pfr-orm
  PUBLIC Boost::pfr PostgreSQL::PostgreSQL
  PRIVATE fmt::fmt)

add_subdirectory(test)
