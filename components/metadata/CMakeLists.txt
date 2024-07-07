add_library(podrm-metadata INTERFACE)

target_compile_features(podrm-metadata INTERFACE cxx_std_20)
target_include_directories(podrm-metadata SYSTEM INTERFACE include)
target_link_libraries(podrm-metadata INTERFACE podrm::span)

add_library(podrm::metadata ALIAS podrm-metadata)

if(BUILD_TESTING)
  # add_subdirectory(test)
endif()
