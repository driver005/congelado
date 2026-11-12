load("@com_google_protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@com_google_protobuf//bazel:proto_library.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")
load("//bazel:copts.bzl", "congelado_copts")

proto_library(
    name = "xplane_proto",
    srcs = ["proto/xplane.proto"],
)

cc_proto_library(
    name = "xplane_cc_proto",
    deps = [":xplane_proto"],
)

# Plain C++ (no named modules): protobuf's generated headers pull in external-repo headers,
# which the module dependency scanner cannot see. The header stays std-only.
cc_library(
    name = "xspace_writer",
    srcs = ["xspace_writer.cc"],
    hdrs = ["xspace_writer.h"],
    copts = congelado_copts() + ["-std=c++23"],
    visibility = ["//visibility:public"],
    deps = [":xplane_cc_proto"],
)

cc_test(
    name = "xspace_writer_test",
    srcs = ["xspace_writer_test.cc"],
    copts = congelado_copts() + ["-std=c++23"],
    deps = [
        ":xplane_cc_proto",
        ":xspace_writer",
    ],
)
