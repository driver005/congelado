# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_random_generator",
    partitions = ["philox_codec.cppm", "philox_state.cppm", "random_generator.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/extern/random_generator/builder:cc_ice_extern_random_generator_builder", "//include/cc/ice/extern/random_generator/sonic:cc_ice_extern_random_generator_sonic", "//include/cc/ice/extern/stream_executor/sonic:cc_ice_extern_stream_executor_sonic", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "@sycl//:sycl"],
)
