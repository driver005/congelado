load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing orchestrator handle (ice::sonic::Orchestrator) + the C-ABI adapter
# it uses to cross into a loaded plugin. Depends on cc_abi_builder_orchestrator for the
# interface type and cc_abi_builder_worker for CompletionFn reuse, plus cc_abi_sonic_registration
# for the in-process factory lookup — never on any specific orchestrator backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_orchestrator",
    primary_interface = "orchestrator.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_orchestrator_hdrs",
        "//include/cc/abi/builder/orchestrator:cc_abi_builder_orchestrator",
        "//include/cc/abi/builder/worker:cc_abi_builder_worker",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
