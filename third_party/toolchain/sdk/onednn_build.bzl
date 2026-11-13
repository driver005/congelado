"""Builds oneDNN from source as an ordinary Bazel action.

The repository rule (onednn.bzl) only downloads the source; the compile runs here, so Bazel shows it as
"Compiling oneDNN", the result lands in the action and disk caches, and an unchanged source, compiler and
argument list is never compiled again, even after the repository is fetched again.

The rule needs no C++ toolchain, so the toolchains themselves can depend on its "ready" stamp: every C++
compile and link action lists that stamp as a tool input, which makes oneDNN build before anything else
compiles. The stamp is a constant file, so rebuilding oneDNN does not invalidate those actions.

Ninja prints its own [done/total] status into a log file while it runs (the path is in the progress
message), because Bazel cannot show progress from inside one action.
"""

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
printf ready > "$ROOT/{stamp}"
echo "oneDNN {vendor}: done" >> "$LOG"
"""

def _impl(ctx):
    compiler = ctx.file.compiler
    compiler_root = compiler.dirname.rsplit("/bin", 1)[0]
    install = ctx.actions.declare_directory(ctx.label.name + "_install")
    library = ctx.actions.declare_file(ctx.label.name + "/libdnnl.so")
    stamp = ctx.actions.declare_file(ctx.label.name + "/ready")
    vendor = ctx.attr.vendor
    log = "/tmp/aten_xpu_onednn_{}.log".format(vendor.lower())

    ctx.actions.run_shell(
        outputs = [install, library, stamp],
        inputs = depset([ctx.file.cmakelists, compiler], transitive = [depset(ctx.files.sources)]),
        command = _SCRIPT.format(
            source = ctx.file.cmakelists.dirname,
            compiler_root = compiler_root,
            install = install.path,
            library = library.path,
            stamp = stamp.path,
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

    return [
        DefaultInfo(files = depset([install, library, stamp])),
        OutputGroupInfo(
            install = depset([install]),
            library = depset([library]),
            ready = depset([stamp]),
        ),
    ]

def _resources(_os_name, _inputs_size):
    return {"cpu": _CLAIM_CPU, "memory": _CLAIM_MEMORY}

onednn_build = rule(
    implementation = _impl,
    attrs = {
        "vendor": attr.string(mandatory = True),
        "cmakelists": attr.label(mandatory = True, allow_single_file = True),
        "sources": attr.label_list(allow_files = True),
        "compiler": attr.label(mandatory = True, allow_single_file = True),
        "cmake_args": attr.string_list(),
    },
)
