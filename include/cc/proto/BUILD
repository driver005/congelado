load("//bazel:proto.bzl", "proto_service")

proto_service(
    name = "address",
    srcs = ["address.proto"],
)

proto_service(
    name = "types",
    srcs = ["types.proto"],
)

proto_service(
    name = "op_def",
    srcs = ["op_def.proto"],
    deps = [
        ":attr_value_proto",
        ":full_type_proto",
        ":resource_handle_proto",
        ":types_proto",
    ],
)

proto_service(
    name = "attr_value",
    srcs = ["attr_value.proto"],
    deps = [
        ":tensor_proto",
        ":tensor_shape_proto",
        ":types_proto",
    ],
)

proto_service(
    name = "api_def",
    srcs = ["api_def.proto"],
    deps = [":attr_value_proto"],
)

proto_service(
    name = "allocation_description",
    srcs = ["allocation_description.proto"],
)

proto_service(
    name = "tensor_shape",
    srcs = ["tensor_shape.proto"],
)

proto_service(
    name = "versions",
    srcs = ["versions.proto"],
)

proto_service(
    name = "graph_debug_info",
    srcs = ["graph_debug_info.proto"],
)

proto_service(
    name = "tensor_description",
    srcs = ["tensor_description.proto"],
    deps = [
        ":allocation_description_proto",
        ":tensor_shape_proto",
        ":types_proto",
    ],
)

proto_service(
    name = "resource_handle",
    srcs = ["resource_handle.proto"],
    deps = [
        ":tensor_shape_proto",
        ":types_proto",
    ],
)

proto_service(
    name = "tensor",
    srcs = ["tensor.proto"],
    deps = [
        ":resource_handle_proto",
        ":tensor_shape_proto",
        ":types_proto",
    ],
)

proto_service(
    name = "tensor_slice",
    srcs = ["tensor_slice.proto"],
    deps = [":tensor_shape_proto"],
)

proto_service(
    name = "node_def",
    srcs = ["node_def.proto"],
    deps = [
        ":attr_value_proto",
        ":full_type_proto",
    ],
)

proto_service(
    name = "function",
    srcs = ["function.proto"],
    deps = [
        ":node_def_proto",
        ":op_def_proto",
        ":attr_value_proto",
    ],
)

proto_service(
    name = "graph",
    srcs = ["graph.proto"],
    deps = [
        ":function_proto",
        ":node_def_proto",
        ":versions_proto",
        ":graph_debug_info_proto",
    ],
)

proto_service(
    name = "cost_graph",
    srcs = ["cost_graph.proto"],
    deps = [
        ":tensor_shape_proto",
        ":types_proto",
    ],
)

proto_service(
    name = "device_attributes",
    srcs = ["device_attributes.proto"],
)

proto_service(
    name = "step_stats",
    srcs = ["step_stats.proto"],
    deps = [
        ":allocation_description_proto",
        ":tensor_description_proto",
    ],
)

proto_service(
    name = "kernel_def",
    srcs = ["kernel_def.proto"],
    deps = [":attr_value_proto"],
)

proto_service(
    name = "summary",
    srcs = ["summary.proto"],
    deps = [
        ":tensor_proto",
        "@xla//xla/tsl/protobuf:histogram_proto",
    ],
)

proto_service(
    name = "variable",
    srcs = ["variable.proto"],
)

proto_service(
    name = "reader_base",
    srcs = ["reader_base.proto"],
)

proto_service(
    name = "full_type",
    srcs = ["full_type.proto"],
)

proto_service(
    name = "log_memory",
    srcs = ["log_memory.proto"],
    deps = [
        ":allocation_description_proto",
        ":tensor_description_proto",
    ],
)
