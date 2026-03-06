load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("//bazel:copts.bzl", "COPTS")

def congelado(name = "congelado", deps = []):
    module_interfaces = native.glob(["include/**/*.cppm"])
    all_headers = native.glob(["include/**/*.h"])

    remote_includes = []
    for h in all_headers:
        path_segments = h.rpartition("/")
        if path_segments[0] and path_segments[0] not in remote_includes:
            remote_includes.append(path_segments[0])
  
    cc_library(
        name = name,
        module_interfaces = module_interfaces,
        hdrs = all_headers,
        srcs = native.glob(["src/**/*.cc"], exclude = ["src/main.cc"]),
        includes = ["include", "src"] + remote_includes,
        linkopts = ["-latomic"],
        copts = COPTS,
        features = ["cpp_modules"],
        deps = [
            "@grpc//:grpc++",
            "@protobuf//:protobuf",
            "@concurrentqueue",
        ] + deps,
        visibility = ["//visibility:public"],
        alwayslink = 1,
    )

    cc_binary(
        name = "congelado_static_exec",
        srcs = ["src/main.cc"],
        copts = COPTS,
        linkstatic = True,
        deps = [":congelado"],
    )

    cc_binary(
        name = "congelado_exec",
        srcs = ["src/main.cc"],
        linkopts = ["-ldl"],
        copts = COPTS,
        deps = [":congelado"]
    )

    test_srcs = native.glob(["tests/**/*.cc"])
    for src in test_srcs:
        test_name = src.rpartition("/")[2].replace(".cc", "_test")
        cc_test(
            name = test_name,
            srcs = [src],
            includes = ["include", "src"] + remote_includes,
            copts = COPTS + ["-fsanitize=thread"],
            linkopts = ["-latomic", "-fsanitize=thread"],
            features = ["cpp_modules"],
            deps = [
                ":" + name,
                "@catch2//:catch2_main",
            ],
        )

    bench_srcs = native.glob(["benchmarks/**/*.cc"])
    for src in bench_srcs:
        bench_name = src.rpartition("/")[2].replace(".cc", "_bench")
        cc_binary(
            name = bench_name,
            srcs = [src],
            includes = ["include", "src"] + remote_includes,
            copts = COPTS,
            linkopts = ["-latomic"],
            features = ["cpp_modules"],
            deps = [
                ":" + name,
                "@catch2//:catch2_main",
            ],
        )
