load("@protobuf//bazel:proto_library.bzl", "proto_library")
load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@rules_cc//cc:defs.bzl", "cc_library")

def proto_service(name, srcs, deps = []):
    # 1. The Proto Library with Virtual Mapping
    proto_library(
        name = name + "_proto",
        srcs = srcs,
        deps = deps,
        import_prefix = "proto", 
        strip_import_prefix = "", 
        visibility = ["//visibility:private"],
    )

    # 2. CC Proto (Messages)
    cc_proto_library(
        name = name + "_cc_proto",
        deps = [":" + name + "_proto"],
        visibility = ["//visibility:private"],
    )

    # 3. CC gRPC (Services)
    cc_grpc_library(
        name = name + "_cc_grpc",
        srcs = [":" + name + "_proto"],
        deps = [":" + name + "_cc_proto"],
        grpc_only = True,
        visibility = ["//visibility:private"],
    )

    # 4. The Library Wrapper
    cc_library(
        name = name + "_lib",
        deps = [
            ":" + name + "_cc_grpc",
            ":" + name + "_cc_proto",
            "@grpc//:grpc++",
            "@protobuf//:protobuf",
        ],
        visibility = ["//visibility:public"],
    )
