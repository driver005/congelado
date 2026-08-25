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
    deps = [
        ":node",
        "//include/utils:consts",
    ],
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "page",
    primary_interface = "page.cppm",
    deps = [
        ":node",
        "//include/utils:consts",
        "//include/utils:helper",
    ],
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "pager",
    primary_interface = "pager.cppm",
    deps = [
        "//include/utils:consts",
        ":page",
        ":atomic_list",
    ],
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "queue",
    primary_interface = "queue.cppm",
    deps = [
        ":pager",
        ":node",
        ":atomic_list",
        "//include/utils:consts",
    ],
    visibility = ["//visibility:public"],
)
