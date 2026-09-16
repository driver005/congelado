# AdaptiveCpp SYCL toolchain (github.com/AdaptiveCpp/AdaptiveCpp), installed on-host at
# /usr/local via its own CMake build — not built from source here, see repo.bzl for why.
#
# Exposes headers/runtime libs so a cc_library/congelado_module_library can depend on
# `@adaptivecpp//:headers` + `@adaptivecpp//:runtime` for host-side SYCL code (USM
# allocation, queue/buffer management, interop_handle/get_native, ...).
#
# Does NOT register a cc_toolchain for device-code compilation: AdaptiveCpp's actual
# CUDA/ROCm/Level-Zero kernel codegen goes through its own `acpp` compiler driver
# (a full clang wrapper doing multi-pass compilation), which isn't a single-cc_toolchain
# shape rules_cc models. A target with actual SYCL kernels (not just host-side SYCL API
# calls) must be compiled by invoking `@adaptivecpp//:acpp` directly (a genrule/cc_binary
# wrapping it), not by adding this as a normal deps= entry.

filegroup(
    name = "acpp",
    srcs = ["bin/acpp"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "headers",
    hdrs = glob(["include/**/*.hpp", "include/**/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "runtime",
    srcs = glob([
        "lib/libacpp-rt.so*",
        "lib/libAdaptiveCpp*.so*",
    ]),
    hdrs = glob(["include/**/*.hpp", "include/**/*.h"]),
    includes = ["include"],
    visibility = ["//visibility:public"],
)
