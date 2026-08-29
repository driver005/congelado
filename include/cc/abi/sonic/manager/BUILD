load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing worker-manager handle (ice::sonic::WorkerManager) + the C-ABI adapter it
# uses to cross into a loaded plugin. The adapter is standalone (no dependency on the
# builder tier); add_worker takes an ice::sonic::Worker, and cc_abi_sonic_registration
# provides the in-process factory lookup — never on any specific manager backend
# implementation.
congelado_module_library(
    name = "cc_abi_sonic_manager",
    primary_interface = "manager.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_manager_hdrs",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
        "//include/cc/abi/sonic/worker:cc_abi_sonic_worker",
        "//include/cc/abi/primitives:cc_abi_primitives",
    ],
    visibility = ["//visibility:public"],
)
