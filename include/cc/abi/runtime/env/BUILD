load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_runtime_env",
    primary_interface = "env.cppm",
    partitions = [
        "dynamic_library.cppm",
        "filesystem/writable_file_handle_view_runtime.cppm",
        "filesystem/string_stream_view_runtime.cppm",
        "filesystem/file_system_runtime.cppm",
        "thread.cppm",
        "time.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
    ],
    visibility = ["//visibility:public"],
)
