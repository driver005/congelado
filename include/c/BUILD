# C ABI layer for congelado.
#
# TensorFlow-style header targets: each cc_library exposes a small, self-contained
# group of headers with precise deps.  The umbrella `c_api_headers` aggregates
# the public API; `tf_c_headers` aggregates everything for C++ module consumers.
#
# include = ["."] adds this package directory (include/c/) to the system include
# path for every dependent, so #include "abi/macros.h" resolves here.  The
# copts "-Iinclude" in //bazel:copts.bzl makes "c/intern/tf_tensor.h" resolve
# to this package too.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_binary.bzl", "cc_binary")
load("@rules_cc//cc:cc_test.bzl", "cc_test")
load("//bazel:copts.bzl", "congelado_copts")
load("//bazel:build_defs.bzl", "congelado_cxx26_copts")

package(default_visibility = ["//visibility:public"])

# =============================================================================
# ABI layer (abi/)
# =============================================================================

cc_library(
    name = "c_api_macros_hdrs",
    hdrs = ["abi/macros.h"],
)

cc_library(
    name = "c_api_hdrs",
    hdrs = ["abi/api.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_attrtype_hdrs",
        ":tf_buffer_hdrs",
        ":tf_datatype_hdrs",
        ":tf_status_hdrs",
        ":tf_tensor_hdrs",
    ],
)

cc_library(
    name = "c_api_experimental_hdrs",
    hdrs = ["abi/api_experimental.h"],
    deps = [
        ":c_api_hdrs",
    ],
)

cc_library(
    name = "platform_hdrs",
    hdrs = ["abi/platform.h"],
)

# =============================================================================
# Intern layer (intern/) — core C types
# =============================================================================

cc_library(
    name = "tf_attrtype_hdrs",
    hdrs = ["intern/tf_attrtype.h"],
)

cc_library(
    name = "tf_datatype_hdrs",
    hdrs = ["intern/tf_datatype.h"],
    deps = [":c_api_macros_hdrs"],
)

cc_library(
    name = "tf_status_hdrs",
    hdrs = ["intern/tf_status.h"],
    deps = [":c_api_macros_hdrs"],
)

# Backend for tf_status_hdrs — previously declarations-only.
cc_library(
    name = "tf_status_impl",
    alwayslink = True,  # force TF_* symbols in for static links (build.cc/exec config)
    srcs = ["intern/tf_status.cc"],
    copts = congelado_copts(),
    deps = [":tf_status_hdrs"],
)

cc_library(
    name = "tf_buffer_hdrs",
    hdrs = ["intern/tf_buffer.h"],
    deps = [":c_api_macros_hdrs"],
)

cc_library(
    name = "tf_shape_hdrs",
    hdrs = ["intern/tf_shape.h"],
    deps = [":c_api_macros_hdrs"],
)

cc_library(
    name = "tf_file_statistics_hdrs",
    hdrs = ["intern/tf_file_statistics.h"],
    deps = [":c_api_macros_hdrs"],
)

cc_library(
    name = "tf_tensor_hdrs",
    hdrs = [
        "intern/tf_tensor.h",
        "intern/tf_tensor_helper.h",
    ],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_datatype_hdrs",
        ":tf_status_hdrs",
    ],
)

cc_library(
    name = "tf_tstring_hdrs",
    hdrs = ["intern/tf_tstring.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_tensor_hdrs",
    ],
)

# Backend for tf_tstring_hdrs — previously declarations-only; a prerequisite for
# ice::String (cc/abi/intern/string.cppm), which the TF_Generator_* cc wrapper uses for its
# string parameters.
cc_library(
    name = "tf_tstring_impl",
    alwayslink = True,  # force TF_* symbols in for static links (build.cc/exec config)
    srcs = ["intern/tf_tstring.cc"],
    copts = congelado_copts(),
    deps = [":tf_tstring_hdrs"],
)

# =============================================================================
# Extern layer — subdirectory headers (extern/<name>/<name>.h)
# =============================================================================

cc_library(
    name = "extern_cache_hdrs",
    hdrs = ["extern/cache/cache.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_cron_hdrs",
    hdrs = ["extern/cron/cron.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_database_hdrs",
    hdrs = ["extern/database/database.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_env_hdrs",
    hdrs = [
        "extern/env/dynamic_library.h",
        "extern/env/env.h",
        "extern/env/thread.h",
    ],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
    ],
)

cc_library(
    name = "extern_events_hdrs",
    hdrs = ["extern/events/events.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_filesystem_hdrs",
    hdrs = [
        "extern/filesystem/filesystem.h",
        "extern/filesystem/option_types.h",
    ],
    deps = [
        ":c_api_macros_hdrs",
    ],
)

cc_library(
    name = "extern_generator_hdrs",
    hdrs = ["extern/generator/generator.h"],
    deps = [
        ":c_api_macros_hdrs",
    ],
)

cc_library(
    name = "extern_io_enums_hdrs",
    hdrs = ["extern/io/enums.h"],
    deps = [":c_api_macros_hdrs"],
)

cc_library(
    name = "extern_io_hdrs",
    hdrs = ["extern/io/io.h"],
    deps = [
        ":c_api_macros_hdrs",
    ],
)

cc_library(
    name = "extern_logger_hdrs",
    hdrs = ["extern/logger/logger.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_manager_hdrs",
    hdrs = ["extern/manager/manager.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":extern_worker_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_orchestrator_hdrs",
    hdrs = ["extern/orchestrator/orchestrator.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":extern_worker_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_otel_enums_hdrs",
    hdrs = ["extern/otel/enums.h"],
)

cc_library(
    name = "extern_otel_hdrs",
    hdrs = ["extern/otel/otel.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":extern_otel_enums_hdrs",
    ],
)

cc_library(
    name = "extern_payload_hdrs",
    hdrs = ["extern/payload/payload.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_plugin_hdrs",
    hdrs = ["extern/plugin/registration.h"],
    deps = [
        ":c_api_macros_hdrs",
    ],
)

cc_library(
    name = "extern_profiler_hdrs",
    hdrs = ["extern/profiler/profiler.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
    ],
)

cc_library(
    name = "extern_protocol_hdrs",
    hdrs = ["extern/protocol/protocol.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_search_hdrs",
    hdrs = ["extern/search/search.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_serde_hdrs",
    hdrs = ["extern/serde/serde.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_worker_hdrs",
    hdrs = ["extern/worker/worker.h"],
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

cc_library(
    name = "extern_registration_hdrs",
    hdrs = [
        "extern/registration/registration.h",
    ],
    srcs = ["extern/registration/registration.cc"],
    copts = congelado_copts(),
    deps = [
        ":c_api_macros_hdrs",
        ":tf_status_hdrs",
        ":tf_tstring_hdrs",
    ],
)

# =============================================================================

# =============================================================================
# Aggregate: c_api_headers — the public TF C API surface
# =============================================================================

cc_library(
    name = "c_api_headers",
    deps = [
        ":c_api_hdrs",
        ":c_api_macros_hdrs",
        ":extern_cache_hdrs",
        ":extern_cron_hdrs",
        ":extern_database_hdrs",
        ":extern_env_hdrs",
        ":extern_events_hdrs",
        ":extern_filesystem_hdrs",
        ":extern_generator_hdrs",
        ":extern_io_hdrs",
        ":extern_logger_hdrs",
        ":extern_manager_hdrs",
        ":extern_orchestrator_hdrs",
        ":extern_otel_hdrs",
        ":extern_payload_hdrs",
        ":extern_plugin_hdrs",
        ":extern_profiler_hdrs",
        ":extern_protocol_hdrs",
        ":extern_registration_hdrs",
        ":extern_search_hdrs",
        ":extern_serde_hdrs",
        ":extern_worker_hdrs",
        ":tf_attrtype_hdrs",
        ":tf_buffer_hdrs",
        ":tf_datatype_hdrs",
        ":tf_file_statistics_hdrs",
        ":tf_shape_hdrs",
        ":tf_status_hdrs",
        ":tf_tensor_hdrs",
        ":tf_tstring_hdrs",
    ],
)

# =============================================================================
# Aggregate: tf_c_headers — everything, for C++ module consumers
# =============================================================================

cc_library(
    name = "tf_c_headers",
    deps = [
        ":c_api_headers",
        ":platform_hdrs",
    ],
)

# =============================================================================
# XLA C API — commented out (XLA dependency removed)
# =============================================================================

# cc_library(
#     name = "xla_pjrt_plugin_entries",
#     hdrs = [
#         "@xla//xla/pjrt/c:pjrt_c_api_cpu.h",
#         "@xla//xla/pjrt/c:pjrt_c_api_gpu.h",
#     ],
# )

# cc_library(
#     name = "xla_c_api",
#     hdrs = ["xla/xla.h"],
#     strip_include_prefix = "/include/c",
#     deps = [
#         ":xla_pjrt_plugin_entries",
#         "@xla//xla/backends/profiler/plugin:profiler_c_api_hdrs",
#         "@xla//xla/c:c_api_decl",
#         "@xla//xla/ffi/api:c_api",
#         "@xla//xla/pjrt/c:pjrt_c_api_abi_version_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_callback_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_collectives_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_custom_partitioner_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_device_event_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_ffi_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_gpu_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_layouts_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_macros",
#         "@xla//xla/pjrt/c:pjrt_c_api_megascale_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_memory_descriptions_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_multi_slice_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_phase_compile_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_profiler_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_raw_buffer_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_shardings_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_stream_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_tpu_constants",
#         "@xla//xla/pjrt/c:pjrt_c_api_tpu_executable_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_tpu_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_tpu_topology_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_triton_extension_hdrs",
#         "@xla//xla/pjrt/c:pjrt_c_api_xla_transform_extension_hdrs",
#         "@xla//xla/tsl/c:tsl_status_headers",
#     ],
# )

# cc_binary(
#     name = "xla_c_api_smoke",
#     srcs = ["xla/smoke.cc"],
#     deps = [":xla_c_api"],
# )

# cc_test(
#     name = "xla_cpu_runtime_test",
#     srcs = ["xla/cpu_runtime_test.cc"],
#     deps = [
#         "@xla//xla/client:client_library",
#         "@xla//xla/service:cpu_plugin",
#         "@xla//xla/hlo/builder:xla_builder",
#         "@xla//xla:literal",
#     ],
# )
