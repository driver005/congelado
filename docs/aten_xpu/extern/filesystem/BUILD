# TEMPORARY local verification target; deleted before commit.
load("//third_party:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "aten_xpu_extern_filesystem",
    partitions = ["filesystem.cppm", "random_access_file.cppm", "read_only_memory_region.cppm", "tree.cppm", "writable_file.cppm"],
    primary_interface = "base.cppm",
    visibility = ["//visibility:public"],
    deps = ["//docs/aten_xpu/intern:aten_xpu_intern", "//include/c:tf_c_headers", "//include/cc/ice/extern/filesystem/builder:cc_ice_extern_filesystem_builder", "//include/cc/ice/extern/filesystem/sonic:cc_ice_extern_filesystem_sonic", "//include/cc/ice/intern/sonic:cc_ice_intern_sonic", "@sycl//:sycl"],
)
