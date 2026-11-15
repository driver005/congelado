load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "lucki_extern_stream_executor",
    partitions = [],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "@level_zero//:headers",
        "//include/c:c_headers",
        "//include/cc/ice/extern/stream_executor:cc_ice_extern_stream_executor",
        "//include/cc/ice/intern/sonic:cc_ice_intern_sonic",
        "@sycl//:sycl",
    ],
)
