load("//bazel:build_defs.bzl", "congelado_module_library")

# Pure search-backend interface — abstract base class (Search) + the SearchQuery value type it
# takes. Zero dependency on any specific backend implementation, which registers a factory
# function pointer into ice::sonic::RegistrationRuntime (cc_abi_sonic_registration) instead.
congelado_module_library(
    name = "cc_abi_builder_search",
    primary_interface = "search.cppm",
    partitions = [
        "search_query.cppm",
        ],
    deps = [
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
    ],
    visibility = ["//visibility:public"],
)
