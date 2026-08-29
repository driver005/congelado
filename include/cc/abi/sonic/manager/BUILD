load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing worker-manager handle (ice::sonic::Manager) + the C-ABI adapter it
# uses to cross into a loaded plugin. Depends on cc_abi_builder_manager for the interface type
# (ice::builder::WorkerManager, which itself depends on cc_abi_builder_worker for the
# Worker type add_worker takes ownership of) and cc_abi_sonic_registration for the in-process
# factory lookup — never on any specific manager backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_manager",
    primary_interface = "manager.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_manager_hdrs",
        "//include/cc/abi/builder/manager:cc_abi_builder_manager",
        "//include/cc/abi/builder/worker:cc_abi_builder_worker",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
