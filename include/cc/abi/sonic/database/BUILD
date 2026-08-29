load("//bazel:build_defs.bzl", "congelado_module_library")

# Mainframe-facing database handle (ice::sonic::Database) + the C-ABI adapter it uses
# to cross into a loaded plugin. Depends on cc_abi_builder_database for the interface type
# (ice::builder::Database) and cc_abi_sonic_registration for the in-process factory
# lookup — never on any specific database backend implementation.
congelado_module_library(
    name = "cc_abi_sonic_database",
    primary_interface = "database.cppm",
    partitions = [
        ],
    deps = [
        "//include/c:extern_database_hdrs",
        "//include/cc/abi/builder/database:cc_abi_builder_database",
        "//include/cc/abi/sonic/intern:cc_abi_sonic_intern",
        "//include/cc/abi/sonic/registration:cc_abi_sonic_registration",
    ],
    visibility = ["//visibility:public"],
)
