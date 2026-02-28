load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("//bazel:copts.bzl", "COPTS")

def congelado(hdrs = [], deps = []):
    cc_library(
        name = "congelado",
        hdrs = hdrs,
        includes = ["include", "src"],
        copts = COPTS,
        deps = [
            "@grpc//:grpc++",
            "@protobuf//:protobuf",
            "@concurrentqueue",
        ] + deps,
        visibility = ["//visibility:public"],
    )

    cc_binary(
        name = "congelado_static_exec",
        srcs = ["src/main.cc"],
        copts = COPTS,
        linkstatic = True,
        deps = [":congelado"],
    )

    cc_binary(
        name = "congelado_exec",
        srcs = ["src/main.cc"],
        linkopts = ["-ldl"],
        copts = COPTS,
        deps = [":congelado"]
    )

