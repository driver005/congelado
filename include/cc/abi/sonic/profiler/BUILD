load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing profiler handle (ice::sonic::Profiler) — always crosses the
# TF_Profiler_* C ABI, deliberately no in-process RegistrationRuntime fast path (see
# controller.cppm's class comment). Depends on cc_abi_builder_profiler for the interface type
# only — never on any specific profiler backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_profiler",
    primary_interface = "profiler.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_profiler_hdrs",
        "//include/cc/abi/builder/profiler:cc_abi_builder_profiler",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
