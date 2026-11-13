load("//bazel:build_defs.bzl", "congelado_module_library")

# helper.cppm is a real partition, imported by stream.cppm but never re-exported by http2.cppm.

congelado_module_library(
    name = "io_layer_http2",
    primary_interface = "http2.cppm",
    partitions = [
        "consts.cppm",
        "executor.cppm",
        "extension.cppm",
        "flow.cppm",
        "frame.cppm",
        "handshake.cppm",
        "helper.cppm",
        "plugin.cppm",
        "req.cppm",
        "res.cppm",
        "session.cppm",
        "settings.cppm",
        "stream.cppm",
    ],
    deps = [
        "//include/shared:shared",
        "//include/core/logger:core_logger",
        "//include/utils/buffering:utils_buffering",
        "//include/io/layer/shared:io_layer_shared",
        "//include/interfaces:interfaces",
        "//include/core/events:core_events",
        "//include/core/contracts:core_contract",
        "//include/io/error:io_error",
        "//include/utils/codec:utils_codec",
        "//include/core/config:core_config",
        "//include/core/router:core_router",
        "//include/io/shared:io_shared",
        "//include/io/base/socket:io_base_socket",
        "//include/io/base/leverage:io_base_leverage",
        "//include/io/flow/socket:io_flow_socket",
        "//include/utils/hashmap:hashmap",
        "//include/io/codec/hpack:io_codec_hpack",
        "//include/utils:utils_encode",
    ],
    visibility = ["//visibility:public"],
)
