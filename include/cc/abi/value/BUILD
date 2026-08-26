load("//bazel:build_defs.bzl", "congelado_module_library")

# Plain value/enum types with no ownership direction — no ctor/dtor tied to a C-allocated
# resource, so there's nothing for a Builder/Runtime split to mean. Shared by both
# cc_abi_builder and cc_abi_sonic.
congelado_module_library(
    name = "cc_abi_value",
    primary_interface = "value.cppm",
    partitions = [
        "allocator_attributes.cppm",
        "attrtype.cppm",
        "datatype.cppm",
        "file_statistics.cppm",
        "filesystem_option_types/filesystem_option_value.cppm",
        "filesystem_option_types/filesystem_option.cppm",
        "io_enums.cppm",
        "otel_enums.cppm",
        "status.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
