"""A stamp that exists once the external C++ dependencies are compiled.

Every own target (congelado_cc_library, congelado_cc_binary, congelado_cc_test, congelado_module_library)
depends on it, so no own code compiles before these dependencies are built. The order of a whole build is
therefore: oneDNN (a tool input of the toolchains, see toolchain/sdk/onednn_build.bzl), then these
dependencies, then the project.

The stamp is a header with constant content that the compile actions of dependents list as an input, so it
makes them wait without invalidating them when a dependency is rebuilt. The rule needs no C++ toolchain,
which keeps it out of the dependencies' own build.
"""

load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")

def _impl(ctx):
    libraries = []
    for dep in ctx.attr.deps:
        for linker_input in dep[CcInfo].linking_context.linker_inputs.to_list():
            for library in linker_input.libraries:
                # One variant per library, so nothing is compiled twice just for the stamp.
                archive = library.static_library or library.pic_static_library or library.dynamic_library
                if archive:
                    libraries.append(archive)

    stamp = ctx.actions.declare_file(ctx.label.name + ".h")
    ctx.actions.run_shell(
        inputs = libraries,
        outputs = [stamp],
        command = 'printf "// the external C++ dependencies are built\\n" > "$1"',
        arguments = [stamp.path],
        mnemonic = "DepsReady",
        progress_message = "Compiled the external C++ dependencies (%d libraries)" % len(libraries),
    )
    return [
        DefaultInfo(files = depset([stamp])),
        CcInfo(compilation_context = cc_common.create_compilation_context(headers = depset([stamp]))),
    ]

deps_ready = rule(
    implementation = _impl,
    attrs = {
        "deps": attr.label_list(providers = [CcInfo]),
    },
)
