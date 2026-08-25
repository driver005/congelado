"""Project-wide copts, mirroring apply_common_layer_settings() in xmake/common.lua."""

load("//bazel:conditionals.bzl", "if_linux", "if_x86_64")

def congelado_copts():
    """Compiler flags every congelado library/binary should use. No -Werror: xmake's own warning set is TODO/commented out."""
    return [
        "-fPIC",
        "-stdlib=libstdc++",  # matches xmake; libc++ 22 lacks std::move_only_function
        "-Iinclude",  # matches xmake's global add_includedirs(core_root/"include")
    ] + if_x86_64(["-mbmi2"]) + if_linux(["-DIOURINGINLINE=inline"])

def congelado_linkopts():
    """Linker flags for the include/ layer's targets (liburing, pthread, dl)."""
    return if_linux(["-luring", "-lpthread", "-ldl"])
