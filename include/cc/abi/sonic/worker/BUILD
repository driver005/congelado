load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing worker handle (ice::sonic::Worker) + the C-ABI adapter it uses to
# cross into a loaded plugin. Depends on cc_abi_builder_worker for the interface type
# (ice::builder::Worker) and cc_abi_sonic_registration for the in-process factory
# lookup — never on any specific worker backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_worker",
    primary_interface = "worker.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_worker_hdrs",
        "//include/cc/abi/builder/worker:cc_abi_builder_worker",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
