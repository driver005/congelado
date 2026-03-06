# Conan dependencies
find_package(fmt REQUIRED)

# === LINK ===
# Link dynamic with your shared lib/executable
set(FMT_DEPS fmt::fmt)
target_link_libraries(${target} PUBLIC ${FMT_DEPS})
if(CONGELADO_BUILD_STATIC_LIB)
  # Link static with your static lib/executable
  target_link_libraries(${target}_static PUBLIC ${FMT_DEPS})
endif()

include(cmake/smidjson.cmake REQUIERD)
include(cmake/grpc.cmake REQUIERD)
