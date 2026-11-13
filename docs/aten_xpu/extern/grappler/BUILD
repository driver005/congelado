# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_grappler",
    partitions = ["activation.cppm", "arithmetic_pass.cppm", "auto_parallel_pass.cppm", "batch_norm_folding_pass.cppm", "capture_status.cppm", "configs.cppm", "const_tensor.cppm", "constant_folding_pass.cppm", "dead_branch_pass.cppm", "debug_stripper_pass.cppm", "dependency_pass.cppm", "device_graph.cppm", "frame_view.cppm", "function_inlining_pass.cppm", "graph_editor.cppm", "graph_index.cppm", "graph_memory.cppm", "grappler.cppm", "layout_pass.cppm", "loop_invariant_motion_pass.cppm", "mixed_precision_pass.cppm", "op_lookup.cppm", "optimizer.cppm", "pass_context.cppm", "pin_to_host_pass.cppm", "pipeline.cppm", "properties.cppm", "pruning_pass.cppm", "recomputation_pass.cppm", "remapper.cppm", "scoped_allocator_pass.cppm", "shape_pass.cppm", "stack_push_removal_pass.cppm", "swapping_pass.cppm", "tensor_bytes.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//docs/aten_xpu/extern/random_generator:aten_xpu_extern_random_generator", "//docs/aten_xpu/extern/stream_executor:aten_xpu_extern_stream_executor", "//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/extern/grappler/builder:cc_ice_extern_grappler_builder", "//include/cc/ice/extern/grappler/sonic:cc_ice_extern_grappler_sonic", "//include/cc/ice/extern/stream_executor/sonic:cc_ice_extern_stream_executor_sonic", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "//include/cc/proto:attr_value", "//include/cc/proto:function", "//include/cc/proto:graph", "//include/cc/proto:node_def", "//include/cc/proto:op_def", "//include/cc/proto:tensor", "//include/cc/proto:tensor_properties", "//include/cc/proto:tensor_shape", "//include/cc/proto:types", "@sycl//:sycl"],
)
