set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

option(PFR_TEST_USE_FIELD_OF "Use pfrorm::FieldOf instead of pfrorm::Field" OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(PFR_TEST_USE_FIELD_OF ON)
endif()

add_executable(test test.cpp simple.cpp nested.cpp)
target_link_libraries(test pfr-orm fmt::fmt)
if(PFR_TEST_USE_FIELD_OF)
  target_compile_definitions(test PRIVATE -DPFR_TEST_USE_FIELD_OF)
endif()
