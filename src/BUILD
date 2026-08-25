load("//bazel:build_defs.bzl", "congelado_cc_binary")

# OpenXLA hello world: builds an add kernel with StableHloBuilder (stablehlo MLIR text),
# compiles and executes it on PJRT's in-process CPU client. See src/openxla.cc.
# Run with:
#   bazel run //src:openxla
congelado_cc_binary(
    name = "openxla",
    srcs = ["openxla.cc"],
    deps = [
        "//include/cc/stable_hlo:cc_stable_hlo",
        "@xla//xla/pjrt/c:pjrt_c_api_cpu",
        "@xla//xla/pjrt/c:pjrt_c_api_hdrs",
    ],
)
