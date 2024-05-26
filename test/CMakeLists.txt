project(podrm.test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)

option(PFR_TEST_USE_FIELD_OF "Use podrm::FieldOf instead of podrm::Field" OFF)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(PFR_TEST_USE_FIELD_OF ON)
endif()

add_executable(${PROJECT_NAME} test.cpp simple.cpp nested.cpp)
target_link_libraries(${PROJECT_NAME} podrm fmt::fmt)
if(PFR_TEST_USE_FIELD_OF)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DPFR_TEST_USE_FIELD_OF)
endif()
