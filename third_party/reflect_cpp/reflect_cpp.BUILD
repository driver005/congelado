load("@rules_cc//cc:defs.bzl", "cc_library")

# Matches Conan's build: JSON (default ON) + TOML (with_toml=True), bundled ctre/yyjson deps.
cc_library(
    name = "reflect_cpp",
    srcs = [
        "src/reflectcpp.cpp",
        "src/reflectcpp_json.cpp",
        "src/reflectcpp_toml.cpp",
        "src/yyjson.c",
    ],
    # src/rfl/**/*.cpp are #include'd (textually) by the unity-build src/reflectcpp*.cpp wrappers, not compiled directly.
    hdrs = glob(["include/**/*.hpp", "include/**/*.h", "src/rfl/**/*.cpp"]),
    includes = [
        "include",
        "include/rfl/thirdparty",
        "src",
    ],
    deps = ["@//third_party/tomlplusplus_shim:toml++"],
    visibility = ["//visibility:public"],
)
