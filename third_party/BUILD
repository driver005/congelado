load("//third_party:deps_ready.bzl", "deps_ready")

# The external C++ dependencies the own targets use and that Bazel compiles from source. They build
# before any own code (see deps_ready.bzl); add a dependency here when a new one is compiled from source.
deps_ready(
    name = "deps_ready",
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_protobuf//:protobuf",
        "@reflect_cpp//:reflect_cpp",
        "@onednn_intel//:onednn",
    ],
)
