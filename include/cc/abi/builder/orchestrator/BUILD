load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure orchestrator-backend interface — abstract base class only. Depends on
# cc_abi_builder_worker for CompletionFn reuse (task completion reported the same way a worker's
# own async operations are); zero dependency on any specific orchestrator backend
# implementation, which registers a factory function pointer into ice::sonic::Registration
# (cc_abi_sonic_registration) instead.
congelado_module_library(
    name = "cc_abi_builder_orchestrator",
    primary_interface = "orchestrator.cppm",
    partitions = [
        ],
    deps = [
        "//include/cc/abi/builder/worker:cc_abi_builder_worker",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    
        "//include/cc/abi/primitives:cc_abi_primitives",
],
    visibility = ["//visibility:public"],
)
