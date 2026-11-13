# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_registration",
    partitions = ["registration.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//docs/aten_xpu/extern/filesystem:aten_xpu_extern_filesystem", "//docs/aten_xpu/extern/gradients:aten_xpu_extern_gradients", "//docs/aten_xpu/extern/grappler:aten_xpu_extern_grappler", "//docs/aten_xpu/extern/kernel:aten_xpu_extern_kernel", "//docs/aten_xpu/extern/profiler/sycl:aten_xpu_profiler_sycl", "//docs/aten_xpu/extern/random_generator:aten_xpu_extern_random_generator", "//docs/aten_xpu/extern/stream_executor:aten_xpu_extern_stream_executor", "//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "@sycl//:sycl"],
)
