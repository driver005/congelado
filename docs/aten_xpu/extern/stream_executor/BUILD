# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_stream_executor",
    partitions = ["device.cppm", "device_guard.cppm", "event.cppm", "executor.cppm", "level_zero.cppm", "mem_pool.cppm", "memory.cppm", "peer_access.cppm", "platform.cppm", "stream.cppm", "stream_executor.cppm", "stream_pool.cppm", "timer.cppm", "allocator/allocator.cppm", "allocator/block.cppm", "allocator/block_pool.cppm", "allocator/expandable_segment.cppm", "allocator/host_cache.cppm", "allocator/ipc_memory.cppm", "allocator/pluggable_allocator.cppm", "allocator/snapshot.cppm", "allocator/stats.cppm", "allocator/trace.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["@level_zero//:headers", "//docs/aten_xpu/extern/random_generator:aten_xpu_extern_random_generator", "//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/extern/stream_executor/builder:cc_ice_extern_stream_executor_builder", "//include/cc/ice/extern/stream_executor/sonic:cc_ice_extern_stream_executor_sonic", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "@sycl//:sycl"],
)
