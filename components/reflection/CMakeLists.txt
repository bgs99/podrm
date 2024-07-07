add_library(podrm-reflection INTERFACE)

add_subdirectory(vendor)

target_compile_features(podrm-reflection INTERFACE cxx_std_20)
target_include_directories(podrm-reflection SYSTEM INTERFACE include)
target_link_libraries(podrm-reflection INTERFACE podrm-metadata podrm-span
                                                 Boost::pfr)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
