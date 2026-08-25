load("@congelado//bazel:build_defs.bzl", "congelado_cc_library")

package(features = ["cpp_modules"])

# The `std` module BMI every congelado module depends on via congelado_module_library.
#
# NOTE: needs an explicit -std >= c++23. The auto-generated crosstool injects a
# global `-std=c++17` (see local_config_cc cxx_flags), which lands BEFORE target
# copts — without this override clang-scan-deps parses std.cc as C++17, drops
# `export module std;`, and every dependent generate-modmap fails with
# "Module not found: std". Own targets dodge this because congelado_module_library
# appends -std=gnu++26 itself; these two targets are the only bare ones.
#
# copts stay explicit/minimal on purpose (macro default congelado_cxx26_copts()
# would drag -Iinclude/-mbmi2/-DIOURINGINLINE into a libstdc++ BMI build); the
# macro is used purely for the features=["cpp_modules"] wiring consistency.
congelado_cc_library(
    name = "std",
    module_interfaces = ["std.cc"],
    copts = ["-stdlib=libstdc++", "-std=gnu++26"],
    visibility = ["//visibility:public"],
)

congelado_cc_library(
    name = "std_compat",
    module_interfaces = ["std.compat.cc"],
    copts = ["-stdlib=libstdc++", "-std=gnu++26"],
    deps = [":std"],
    visibility = ["//visibility:public"],
)
