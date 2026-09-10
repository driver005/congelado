load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

# Alternative to `bazel-compile-commands //...` (the `make editor` target) -
# that standalone binary collapses multi-source `module_interfaces` targets
# to one wrong entry (see MODULE.bazel's hedron_compile_commands comment).
# Run: bazel run //:refresh_compile_commands
refresh_compile_commands(
    name = "refresh_compile_commands",
    targets = ["//..."],
)
