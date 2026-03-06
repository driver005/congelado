# 1. Set experimental flag BEFORE project()
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

if(NOT CONAN_PROVIDER_LOADED)
  set(CONAN_PROVIDER_LOADED ON)
  include("${CMAKE_CURRENT_LIST_DIR}/../out/conan_provider.cmake")
endif()
