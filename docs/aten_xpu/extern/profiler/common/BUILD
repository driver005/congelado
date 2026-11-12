load("//bazel:build_defs.bzl", "congelado_module_library")

# Everything the four profilers share that does not need SYCL: the tracer interface, the XSpace
# producing profiler base, a host-memory tensor and the plugin owner.
congelado_module_library(
    name = "aten_xpu_profiler_common",
    hdrs = ["vendor_entry.h"],
    partitions = [
        "kind.cppm",
        "dynamic_library.cppm",
        "device_activity.cppm",
        "backend.cppm",
        "host_ops.cppm",
        "status.cppm",
        "host_tensor.cppm",
        "vendor_profiler.cppm",
        "vendor_plugin.cppm",
    ],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//docs/aten_xpu/extern/profiler:xspace_writer",
        "//include/c:tf_c_headers",
        "//include/cc/ice/extern/profiler/builder:cc_ice_extern_profiler_builder",
        "//include/cc/ice/intern/builder:cc_ice_intern_builder",
        "//include/cc/ice/intern/sonic:cc_ice_intern_sonic",
    ],
)
