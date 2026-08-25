load("//bazel:build_defs.bzl", "congelado_module_library")

congelado_module_library(
    name = "cc_abi_builder_env",
    primary_interface = "env.cppm",
    partitions = [
        "dynamic_library.cppm",
        "filesystem/writable_file_handle_view.cppm",
        "filesystem/string_stream_view.cppm",
        "filesystem/file_system.cppm",
        "thread/thread_options_builder.cppm",
        "thread/thread_view.cppm",
        "time.cppm",
    ],
    deps = [
        "//include/c:tf_c_headers",
        "//include/cc/abi/builder/intern:cc_abi_builder_intern",
    ],
    visibility = ["//visibility:public"],
)
