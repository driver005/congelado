load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_primitives",
    primary_interface = "primitives.cppm",
    partitions = [
        "allocator_attributes.cppm",
        "file_statistics.cppm",
        "filesystem_option.cppm",
        "status.cppm",
        "string.cppm",
        "string_hive.cppm",
        "tensor_handle.cppm",
        "types.cppm",
    ],
    deps = [
        "//include/c:extern_filesystem_hdrs",
        "//include/c:extern_io_enums_hdrs",
        "//include/c:extern_otel_enums_hdrs",
        "//include/c:extern_payload_hdrs",
        "//include/c:tf_datatype_hdrs",
        "//include/c:tf_file_statistics_hdrs",
        "//include/c:tf_status_hdrs",
        "//include/c:tf_tstring_hdrs",
        "//include/c:tf_tensor_hdrs",
    ],
    visibility = ["//visibility:public"],
)
