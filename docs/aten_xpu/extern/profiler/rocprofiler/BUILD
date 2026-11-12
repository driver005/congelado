load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//bazel:build_defs.bzl", "congelado_cxx26_copts", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_profiler_rocprofiler",
    # The HIP headers pulled in by rocprofiler-sdk refuse to compile without a platform macro.
    copts = congelado_cxx26_copts() + ["-D__HIP_PLATFORM_AMD__"],
    partitions = ["backend.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//docs/aten_xpu/extern/profiler/common:aten_xpu_profiler_common",
        "@rocprofiler//:headers",
    ],
)

# The plugin library the host loads: exports create_profiler / create_tensor. Host symbols
# (create_status, create_string) stay undefined and resolve when the host loads it.
cc_binary(
    name = "libaten_xpu_profiler_rocprofiler.so",
    srcs = ["plugin.cc"],
    copts = congelado_cxx26_copts(),
    features = ["cpp_modules"],
    linkshared = True,
    deps = [":aten_xpu_profiler_rocprofiler"],
)
