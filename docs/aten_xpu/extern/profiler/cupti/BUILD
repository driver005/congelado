load("@rules_cc//cc:defs.bzl", "cc_binary")
load("//third_party:build_defs.bzl", "congelado_cxx26_copts", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_profiler_cupti",
    partitions = ["backend.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = [
        "//docs/aten_xpu/extern/profiler/common:aten_xpu_profiler_common",
        "@cupti//:headers",
    ],
)

# The plugin library the host loads: exports create_profiler / create_tensor. Host symbols
# (create_status, create_string) stay undefined and resolve when the host loads it.
cc_binary(
    name = "libaten_xpu_profiler_cupti.so",
    srcs = ["plugin.cc"],
    copts = congelado_cxx26_copts(),
    features = ["cpp_modules"],
    linkshared = True,
    deps = [":aten_xpu_profiler_cupti"],
)
