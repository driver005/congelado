load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)

configure_make(
    name = "ucx",
    lib_source = ":all_srcs",
    configure_options = [
        "--disable-doxygen-doc",
        "--disable-static",
        "--enable-shared",
        "--without-java",
        "--without-go",
    ],
    out_shared_libs = [
        "libucp.so",
        "libucs.so",
        "libuct.so",
        "libucm.so",
    ],
    visibility = ["//visibility:public"],
)
