# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_gradients",
    partitions = ["value.cppm", "emitter.cppm", "gradient.cppm", "activation/bias_add.cppm", "activation/relu.cppm", "activation/sparse_softmax_cross_entropy.cppm", "attention/attention.cppm", "convolution/convolution.cppm", "copy/copy.cppm", "math/add.cppm", "math/div_no_nan.cppm", "math/exp.cppm", "math/identity_n.cppm", "math/log1p.cppm", "math/multiply.cppm", "math/negate.cppm", "math/not_differentiable.cppm", "math/sqrt.cppm", "math/subtract.cppm", "matmul/addmm.cppm", "matmul/linear.cppm", "matmul/matmul.cppm", "transpose/transpose.cppm", "catalog.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/extern/gradients/builder:cc_ice_extern_gradients_builder", "//include/cc/ice/extern/gradients/sonic:cc_ice_extern_gradients_sonic", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "@sycl//:sycl"],
)
