cmake_minimum_required(VERSION 3.7)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(podrm LANGUAGES C CXX)

add_subdirectory(components)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  option(PFR_ORM_ASAN "Build podrm with address sanitizer" OFF)

  if(PFR_ORM_ASAN)
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
  endif()

  include(CTest)
endif()
