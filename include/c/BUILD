load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "tf_c_headers",
    hdrs = glob(["**/*.h"]),
)

alias(
    name = "c_api_headers",
    actual = ":tf_c_headers",
)
