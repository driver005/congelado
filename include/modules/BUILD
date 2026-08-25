load("//bazel:build_defs.bzl", "congelado_cc_library", "congelado_module_library")

# Platform-specific system-header-wrapper modules; errno.cppm excluded (1-byte unimplemented stub).

# --- POSIX wrappers ---

congelado_module_library(
    name = "modules.fcntl",
    primary_interface = "fcntl.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "modules.net",
    primary_interface = "net.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "modules.netdb",
    primary_interface = "netdb.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "modules.socket",
    primary_interface = "socket.cppm",
    visibility = ["//visibility:public"],
)

congelado_module_library(
    name = "modules.unistd",
    primary_interface = "unistd.cppm",
    visibility = ["//visibility:public"],
)

# --- Windows-only ---

# wraps <winsock2.h>, not buildable/relevant on Linux
congelado_cc_library(
    name = "modules.winsock2",
    module_interfaces = ["winsock2.cppm"],
    deps = ["@system_libstdcxx//:std"],
    target_compatible_with = ["@platforms//os:windows"],
    visibility = ["//visibility:public"],
)

# --- Blocked on third-party deps ---

# asio/openssl modules need those third-party deps wired first (both in BCR, not yet done).
