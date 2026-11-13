# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_intern",
    partitions = ["array.cppm", "attr_type.cppm", "bit_set.cppm", "buffer.cppm", "complex.cppm", "data_type.cppm", "datatype.cppm", "deque.cppm", "duration.cppm", "failure.cppm", "file_statistics.cppm", "forward_list.cppm", "graph_state.cppm", "hash.cppm", "hive.cppm", "list.cppm", "map.cppm", "ops_table.cppm", "placement.cppm", "profile_activity.cppm", "profile_collector.cppm", "profile_memory_event.cppm", "registration.cppm", "set.cppm", "shape.cppm", "shared_state.cppm", "span.cppm", "status.cppm", "status_store.cppm", "string.cppm", "tensor.cppm", "tensor_factory.cppm", "text_tensor.cppm", "time_point.cppm", "vector.cppm"],
    primary_interface = "base.cppm",
    features = ["cpp_modules", "sycl"],
    visibility = ["//visibility:public"],
    deps = [
        "//docs/aten_xpu/extern/profiler/common:aten_xpu_profiler_common",
        "//include/c:tf_c_headers",
        "//include/cc/ice/extern/stream_executor/sonic:cc_ice_extern_stream_executor_sonic",
        "//include/cc/ice/intern/builder:cc_ice_intern_builder",
        "//include/cc/ice/intern/forward_list/builder:cc_ice_intern_forward_list_builder",
        "//include/cc/ice/intern/hive/builder:cc_ice_intern_hive_builder",
        "//include/cc/ice/intern/list/builder:cc_ice_intern_list_builder",
        "//include/cc/ice/intern/sonic:cc_ice_intern_sonic",
        "@sycl//:sycl",
    ],
)
