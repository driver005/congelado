load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_sonic_registration",
    primary_interface = "registration.cppm",
    partitions = [
        "registration_runtime.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/primitives:cc_abi_primitives",
    ],
    visibility = ["//visibility:public"],
)
