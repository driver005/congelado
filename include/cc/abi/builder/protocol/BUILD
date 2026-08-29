load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure protocol-backend interface — abstract base classes only (Protocol + the Server it
# produces via create_server()). Zero dependency on any specific backend implementation, which
# registers a factory function pointer into ice::sonic::RegistrationRuntime
# (cc_abi_sonic_registration) instead.
congelado_module_library(
    name = "cc_abi_builder_protocol",
    primary_interface = "protocol.cppm",
    partitions = [
        ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
