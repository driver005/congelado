list(PREPEND CMAKE_PREFIX_PATH ${CONAN_LIB_DIRS} ${CONAN_BIN_DIRS})

# 2. Use CONFIG mode to ensure we find the generated .cmake files from Conan
# and NOT the FindProtobuf.cmake module from /usr/share/cmake
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Verification: Print the path to be sure
message(STATUS "Using Protobuf from: ${Protobuf_DIR}")
message(STATUS "Using gRPC from: ${gRPC_DIR}")

#
# Protobuf/Grpc source files
#
set(PROTO_FILES
  ${PROJECT_SOURCE_DIR}/proto/address.proto
)

# Add libary
add_library(protolib ${PROTO_FILES})

target_link_libraries(protolib
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)

# === LINK ===
# Link dynamic with your shared lib/executable
set(GRPC_DEPS protolib)
target_link_libraries(${target} PUBLIC ${GRPC_DEPS})
if(CONGELADO_BUILD_STATIC_LIB)
    # Link static with your static lib/executable
    target_link_libraries(${target}_static PUBLIC ${GRPC_DEPS})
endif()


# Add Library target with protobuf sources
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


# Compile protobuf and grpc files in protolib target to cpp
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(TARGET protolib LANGUAGE cpp)
protobuf_generate(TARGET protolib LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
