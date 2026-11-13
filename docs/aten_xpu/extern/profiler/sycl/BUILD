# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_profiler_sycl",
    partitions = ["profiler.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//docs/aten_xpu/extern/stream_executor:aten_xpu_extern_stream_executor",
        "//docs/aten_xpu/intern:aten_xpu_intern",
        "//include/c:tf_c_headers",
        "//include/cc/ice/extern/profiler/builder:cc_ice_extern_profiler_builder",
        "//include/cc/ice/extern/stream_executor/sonic:cc_ice_extern_stream_executor_sonic",
        "//include/cc/ice/intern/sonic:cc_ice_intern_sonic",
        "//docs/aten_xpu/extern/profiler:xspace_writer",
        "@sycl//:sycl",
    ],
)
