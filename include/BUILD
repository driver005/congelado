load("//bazel:build_defs.bzl", "congelado_cc_library", "congelado_module_library")

# --- Module ---

# congelado.cppm: umbrella module, re-exports whole interfaces and shared modules.

congelado_module_library(
    name = "congelado",
    primary_interface = "congelado.cppm",
    deps = [
        "//include/cc:cc",
        "//include/interfaces:interfaces",
        "//include/shared:shared",
    ],
    visibility = ["//visibility:public"],
)

# --- Aggregate ---

# Aggregate mirroring xmake's congelado_include, a thin wrapper not a recompilation.
congelado_cc_library(
    name = "congelado_include",
    deps = [
        ":congelado",
        "//include/connector:connector",
        "//include/core/client:core_client",
        "//include/core/config:core_config",
        "//include/core/contracts:core_contract",
        "//include/core/events:core_events",
        "//include/core/ffi:core_ffi",
        "//include/core/generator:core_generator",
        "//include/core/logger:core_logger",
        "//include/core/manager:core_plugin",
        "//include/core/otel:core_otel",
        "//include/core/router:core_router",
        "//include/interfaces:interfaces",
        "//include/io/base/leverage:io_base_leverage",
        "//include/io/base/socket:io_base_socket",
        "//include/io/codec/hpack:io_codec_hpack",
        "//include/io/codec/qpack:io_codec_qpack",
        "//include/io/codec/quic:io_quic",
        "//include/io/codec/shared:io_codec_shared",
        "//include/io/error:io_error",
        "//include/io/flow:io_base_flow",
        "//include/io/flow/receiver:io_flow_receiver",
        "//include/io/flow/sender:io_flow_sender",
        "//include/io/flow/socket:io_flow_socket",
        "//include/io/layer/http2:io_layer_http2",
        "//include/io/layer/shared:io_layer_shared",
        "//include/io/shared:io_shared",
        "//include/migration:migration",
        "//include/serde:serde",
        "//include/shared:shared",
        "//include/utils:consts",
        "//include/utils:helper",
        "//include/utils:utils_encode",
        "//include/utils:utils_errno_translator",
        "//include/utils/buffering:utils_buffering",
        "//include/utils/codec:utils_codec",
        "//include/utils/hash:utils_hash",
        "//include/utils/hashmap:hashmap",
        "//include/utils/openapi:utils_openapi",
        "//include/utils/queue:atomic_list",
        "//include/utils/queue:node",
        "//include/utils/queue:page",
        "//include/utils/queue:pager",
        "//include/utils/queue:queue",
        # modules/ and plugin/ excluded: unwired third-party deps / empty stub
    ],
    visibility = ["//visibility:public"],
)
