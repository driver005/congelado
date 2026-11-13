# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_kernel",
    partitions = ["kernel.cppm", "registrar.cppm", "runtime.cppm", "onednn/convolution_geometry.cppm", "onednn/convolution_primitive.cppm", "onednn/engine_cache.cppm", "onednn/fusion.cppm", "onednn/matmul_primitive.cppm", "onednn/memory_layout.cppm", "onednn/post_op.cppm", "onednn/post_op_attributes.cppm", "onednn/primitive_cache.cppm", "onednn/primitive_executor.cppm", "onednn/quantization.cppm", "onednn/runtime.cppm", "context/construction.cppm", "context/context.cppm", "linear/linear.cppm", "recurrent/lstm.cppm", "matmul/addmm.cppm", "matmul/addmv.cppm", "matmul/baddbmm.cppm", "matmul/bmm.cppm", "matmul/scaled_add.cppm", "convolution/conv.cppm", "convolution/conv_backward.cppm", "convolution/deconv.cppm", "quantized/int8_conv.cppm", "quantized/int8_matmul.cppm", "quantized/woq_matmul.cppm", "scaled/scaled_mm.cppm", "scaled/scaling_recipe.cppm", "attention/onednn_sdpa.cppm", "attention/sdpa.cppm", "attention/sdpa_backward.cppm", "attention/sdpa_math.cppm", "attention/sdpa_selector.cppm", "ops/catalog.cppm", "ops/registrar.cppm", "ops/shape_inference.cppm", "transpose/transpose.cppm", "copy/copy_from_host.cppm", "copy/copy_to_host.cppm", "scoped/scoped_allocator.cppm", "scoped/scoped_allocator_concat.cppm", "scoped/scoped_allocator_split.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//docs/aten_xpu/extern/stream_executor:aten_xpu_extern_stream_executor", "//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/extern/kernel/builder:cc_ice_extern_kernel_builder", "//include/cc/ice/extern/kernel/sonic:cc_ice_extern_kernel_sonic", "//include/cc/ice/extern/ops/sonic:cc_ice_extern_ops_sonic", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "@onednn_intel//:onednn", "@sycl//:sycl"],
)
