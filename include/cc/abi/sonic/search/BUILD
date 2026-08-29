load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing search handle (ice::sonic::Search) + the C-ABI adapter it uses to
# cross into a loaded plugin. The adapter is standalone (no dependency on the builder
# tier; ice::SearchQuery comes from cc_abi_primitives); cc_abi_sonic_registration
# provides the in-process factory lookup — never on any specific search backend
# implementation.
congelado_module_library(
    name = "cc_abi_sonic_search",
    primary_interface = "search.cppm",
    partitions = [],
    deps = [
        "//include/c:extern_search_hdrs",
        "//include/cc/abi/primitives:cc_abi_primitives",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
