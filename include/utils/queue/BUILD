load("//bazel:build_defs.bzl", "congelado_module_library")

# Five independent single-file modules co-located here, not partitions of one module.

congelado_module_library(
    name = "node",
    primary_interface = "node.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "atomic_list",
    primary_interface = "atomic_list.cppm",
    visibility = ["//visibility:public"],
    deps = [
        ":node",
        "//include/utils:consts",
    ],
)

congelado_module_library(
    name = "page",
    primary_interface = "page.cppm",
    visibility = ["//visibility:public"],
    deps = [
        ":node",
        "//include/utils:consts",
        "//include/utils:helper",
    ],
)

congelado_module_library(
    name = "pager",
    primary_interface = "pager.cppm",
    visibility = ["//visibility:public"],
    deps = [
        ":atomic_list",
        ":page",
        "//include/utils:consts",
    ],
)

congelado_module_library(
    name = "queue",
    primary_interface = "queue.cppm",
    visibility = ["//visibility:public"],
    deps = [
        ":atomic_list",
        ":node",
        ":pager",
        "//include/utils:consts",
    ],
)
