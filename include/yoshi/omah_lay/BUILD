# Internal plugins implementation layer for Congelado.
#
# These plugins implement the C ABIs declared in //include/c.
# Bazel's action cache ensures targets here are ONLY rebuilt if //include/c
# (or the plugin's own source code) changes.
#
# Approach A (Per-Plugin Choice):
# Each plugin decides its own dependencies:
# - Standard/CPU plugins depend only on `:plugin_c_abi_base`.
# - GPU plugins explicitly add `:gpu_plugin_deps` (or @adaptive_cpp//:sycl
#   and //third_party/libxfile:libxfile) to their deps.
#
# Because Bazel prunes unused dependencies, SYCL and libxFile are NEVER
# built unless a target depending on them is specifically requested!

load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

# Base dependency for all omah_lay plugins: strictly tracks C ABI changes.
cc_library(
    name = "plugin_c_abi_base",
    deps = [
        "//include/c:c_api_headers",
        "//include/c:tf_c_headers",
    ],
)

# Optional GPU stack: plugins that choose GPU acceleration add this to deps.
cc_library(
    name = "gpu_plugin_deps",
    deps = [
        "@adaptive_cpp//:sycl",
        "//third_party/libxfile:libxfile",
    ],
)

# Umbrella target aggregating internal plugins
cc_library(
    name = "omah_lay_plugins",
    deps = [
        ":plugin_c_abi_base",
    ],
)
