load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "sycl",
    hdrs = glob([
        "include/**/*.hpp",
        "include/**/*.h",
    ]),
    includes = [
        "include",
        "include/SYCL",
    ],
)
