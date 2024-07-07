project(podrm-odbc.test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

option(PODRM_TEST_USE_FIELD_OF "Use podrm::FieldOf instead of podrm::Field" OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(PODRM_TEST_USE_FIELD_OF ON)
endif()

if(PODRM_TEST_USE_FIELD_OF)
  add_compile_definitions(-DPODRM_TEST_USE_FIELD_OF)
endif()

find_package(Catch2 3 REQUIRED)

add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} podrm-odbc podrm-reflection fmt::fmt
                      Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
