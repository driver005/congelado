load("//bazel:build_defs.bzl", "congelado_cc_library")

# Throwaway Phase-1 risk probe, not part of the real module graph — see gmf_probe.cppm.

congelado_cc_library(
    name = "gmf_probe",
    module_interfaces = ["gmf_probe.cppm"],
    hdrs = [
        "//include/congelado:headers",
        "//include/core/manager:headers",
    ],
    deps = ["@system_libstdcxx//:std"],
)
