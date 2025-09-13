find_package(simdjson REQUIRED)

# === LINK ===
# Link dynamic with your shared lib/executable
set(FMT_DEPS simdjson::simdjson)
target_link_libraries(${target} PUBLIC ${FMT_DEPS})
if(CONGELADO_BUILD_STATIC_LIB)
  # Link static with your static lib/executable
  target_link_libraries(${target}_static PUBLIC ${FMT_DEPS})
endif()