load("//bazel:build_defs.bzl", "congelado_module_library")

genrule(
    name = "cc_templating_templates_gen",
    srcs = glob(["templates/*.inja"]),
    outs = ["generated/cc_templating_templates.h"],
    cmd = "python3 $(location //bazel:embed_text.py) $@ $(SRCS)",
    tools = ["//bazel:embed_text.py"],
)

congelado_module_library(
    name = "cc_templating",
    primary_interface = "templating.cppm",
    hdrs = [":cc_templating_templates_gen"],
    visibility = ["//visibility:public"],
    deps = [
        "@inja",
        "@nlohmann_json",
    ],
)
