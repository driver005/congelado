"""Builds oneDNN from source as an ordinary Bazel action.

The repository rule (onednn.bzl) only downloads the source; the compile runs here, so Bazel shows it as
"Compiling oneDNN", the result lands in the action and disk caches, and an unchanged source, compiler and
argument list is never compiled again, even after the repository is fetched again.

Ninja prints its own [done/total] status into a log file while it runs (the path is in the progress
message), because Bazel cannot show progress from inside one action.
"""

load("@rules_cc//cc:find_cc_toolchain.bzl", "find_cpp_toolchain", "use_cpp_toolchain")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

# The compile claims the whole machine: Bazel scales a request above the host total down to it, so
# this action runs with nothing else beside it, and ninja then uses every core (HOST_CPUS).
_CLAIM_CPU = 1024
_CLAIM_MEMORY = 1048576

_SCRIPT = """
set -euo pipefail
ROOT="$(pwd)"
SOURCE="$ROOT/{source}"
COMPILER_ROOT="$ROOT/{compiler_root}"
INSTALL="$ROOT/{install}"
LOG="{log}"
BUILD="$(mktemp -d)"
trap 'rm -rf "$BUILD"' EXIT
mkdir -p "$(dirname "$LOG")"
: > "$LOG"
export LD_LIBRARY_PATH="$COMPILER_ROOT/lib"
export NINJA_STATUS="[%f/%t %es] "
echo "oneDNN {vendor}: configure" >> "$LOG"
cmake -S "$SOURCE" -B "$BUILD" -G Ninja \\
    -DCMAKE_BUILD_TYPE=Release \\
    -DCMAKE_INSTALL_PREFIX="$INSTALL" \\
    -DCMAKE_C_COMPILER="$COMPILER_ROOT/bin/clang" \\
    -DCMAKE_CXX_COMPILER="$COMPILER_ROOT/bin/clang++" \\
    {arguments} >> "$LOG" 2>&1 || {{ tail -40 "$LOG" >&2; exit 1; }}
echo "oneDNN {vendor}: compile" >> "$LOG"
cmake --build "$BUILD" -j "$(nproc)" >> "$LOG" 2>&1 || {{ tail -40 "$LOG" >&2; exit 1; }}
echo "oneDNN {vendor}: install" >> "$LOG"
cmake --install "$BUILD" >> "$LOG" 2>&1 || {{ tail -40 "$LOG" >&2; exit 1; }}
cp -L "$INSTALL/lib/libdnnl.so" "$ROOT/{library}"
echo "oneDNN {vendor}: done" >> "$LOG"
"""

def _impl(ctx):
    compiler = ctx.file.compiler
    compiler_root = compiler.dirname.rsplit("/bin", 1)[0]
    install = ctx.actions.declare_directory(ctx.label.name + "_install")
    library = ctx.actions.declare_file(ctx.label.name + "/libdnnl.so")
    vendor = ctx.attr.vendor
    log = "/tmp/aten_xpu_onednn_{}.log".format(vendor.lower())

    ctx.actions.run_shell(
        outputs = [install, library],
        inputs = depset([ctx.file.cmakelists, compiler], transitive = [depset(ctx.files.sources)]),
        command = _SCRIPT.format(
            source = ctx.file.cmakelists.dirname,
            compiler_root = compiler_root,
            install = install.path,
            library = library.path,
            log = log,
            vendor = vendor,
            arguments = " ".join(["'{}'".format(argument) for argument in ctx.attr.cmake_args]),
        ),
        mnemonic = "OneDnnBuild",
        progress_message = "Compiling oneDNN {} with {} (ninja status: tail -f {})".format(vendor, compiler.basename, log),
        use_default_shell_env = True,
        execution_requirements = {"no-sandbox": "1", "local": "1"},
        resource_set = _resources,
    )

    toolchain = find_cpp_toolchain(ctx)
    features = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = toolchain,
        requested_features = ctx.features,
        unsupported_features = ctx.disabled_features,
    )
    to_link = cc_common.create_library_to_link(
        actions = ctx.actions,
        feature_configuration = features,
        cc_toolchain = toolchain,
        dynamic_library = library,
    )
    return [
        DefaultInfo(files = depset([install, library]), runfiles = ctx.runfiles(files = [library])),
        CcInfo(
            compilation_context = cc_common.create_compilation_context(
                headers = depset([install]),
                system_includes = depset([install.path + "/include"]),
            ),
            linking_context = cc_common.create_linking_context(
                linker_inputs = depset([cc_common.create_linker_input(
                    owner = ctx.label,
                    libraries = depset([to_link]),
                )]),
            ),
        ),
    ]

def _resources(_os_name, _inputs_size):
    return {"cpu": _CLAIM_CPU, "memory": _CLAIM_MEMORY}

onednn_cmake = rule(
    implementation = _impl,
    attrs = {
        "vendor": attr.string(mandatory = True),
        "cmakelists": attr.label(mandatory = True, allow_single_file = True),
        "sources": attr.label_list(allow_files = True),
        "compiler": attr.label(mandatory = True, allow_single_file = True),
        "cmake_args": attr.string_list(),
    },
    fragments = ["cpp"],
    toolchains = use_cpp_toolchain(),
)
