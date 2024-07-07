option(PODRM_USE_GSL_SPAN
       "Use Microsoft.GSL for span implementation instead of std::span" OFF)
add_library(podrm-span INTERFACE)
if(PODRM_USE_GSL_SPAN)
  target_compile_definitions(podrm-span INTERFACE PODRM_USE_GSL_SPAN)
  find_package(Microsoft.GSL REQUIRED)
  target_link_libraries(podrm-span INTERFACE Microsoft.GSL::GSL)
endif()
target_include_directories(podrm-span INTERFACE SYSTEM include)

add_library(podrm::span ALIAS podrm-span)
