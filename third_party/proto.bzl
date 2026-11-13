"""Proto library helpers for congelado."""

load("@com_google_protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

def proto_service(name, srcs, deps = [], visibility = None):
    """Creates a proto_library + cc_proto_library pair."""
    proto_name = name + "_proto"
    proto_library(
        name = proto_name,
        srcs = srcs,
        deps = deps,
        visibility = visibility,
        import_prefix = "cc/proto",
        strip_import_prefix = "",
    )
    cc_proto_library(
        name = name,
        deps = [":" + proto_name],
        visibility = visibility,
    )
