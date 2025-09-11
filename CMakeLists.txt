cmake_minimum_required(VERSION 3.10.0)
project(congelado VERSION 0.1.0 LANGUAGES C CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# === TOOLCHAIN ===
message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
# For a truly reproducible build, you would use a separate file:
# cmake -DCMAKE_TOOLCHAIN_FILE=path/to/toolchain.cmake ..
# OR LOOK INTO "CMakePresets.json"

include(cmake/options.cmake)
include(cmake/props.cmake)
include(cmake/cache.cmake)
include(cmake/conan.cmake)

# === GLOBAL VARS ===
set(target congelado)  
set(SOURCES src/main.cc)

# === GENERATE LIBRARY ===
add_library(${target} SHARED ${SOURCES})
add_library(${target}::${target} ALIAS ${target})

if(CONGELADO_BUILD_STATIC_LIB)
  add_library(${target}_static STATIC ${SOURCES})
  add_library(congelado::${target}_static ALIAS ${target}_static)
endif()

# === ADD PROJECT CODE ===
add_props(
  target_include_directories
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
  PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

# === COMPILER FLAGS ===
target_compile_options(${target} PRIVATE -Wall -Wextra -Werror)
if(CONGELADO_BUILD_STATIC_LIB)
  target_compile_options(${target}_static PRIVATE -Wall -Wextra -Werror)
endif()

# === REPRODUCIBLITY ===
set(CMAKE_REPRODUCIBLE TRUE)
set(CMAKE_SOURCE_DATE_EPOCH 946684800)
add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=. -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=.)


# === EXECUTABLE ===
add_executable(${target}_exec ${SOURCES})

if(CONGELADO_BUILD_STATIC_LIB)
  target_link_libraries(${target}_exec PRIVATE ${target}::${target}_static)

  # Create a fully staticlly linked executable for all lazy ass people 
  add_executable(${target}_static_exec ${SOURCES})
  target_link_libraries(${target}_static_exec PRIVATE ${target}::${target}_static)
  target_link_options(${target}_static_exec PRIVATE -static)
else()
  target_link_libraries(${target}_exec PRIVATE ${target}::${target})
endif()


# === APPLY CODE ===
apply_props(${target})
if(CONGELADO_BUILD_STATIC_LIB)
  apply_props(${target}_static)
endif()

# include(CTest)
# enable_testing()

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# include(CPack)
