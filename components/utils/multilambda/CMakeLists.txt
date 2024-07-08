add_library(podrm-multilambda INTERFACE)
target_include_directories(podrm-multilambda INTERFACE SYSTEM include)

add_library(podrm::multilambda ALIAS podrm-multilambda)
