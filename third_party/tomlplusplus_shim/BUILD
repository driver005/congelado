load("@rules_cc//cc:defs.bzl", "cc_library")

# BCR's tomlplusplus exposes a flat toml.hpp; congelado needs it under toml++/ — copy, don't redirect (redirect self-matches).
genrule(
    name = "toml_hpp_copy",
    srcs = ["@tomlplusplus//:toml.hpp"],
    outs = ["toml++/toml.hpp"],
    cmd = "cp $< $@",
)

cc_library(
    name = "toml++",
    hdrs = [":toml_hpp_copy"],
    includes = ["."],
    visibility = ["//visibility:public"],
)
