"""Custom build macros modeled on TensorFlow's tensorflow.bzl (tf_copts, tf_cuda_library, check_deps), adapted not copied."""

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")
load("//bazel:copts.bzl", "congelado_copts", "congelado_linkopts")

def congelado_cxx26_copts():
    """Default copts for all own targets. C++26 std is set per-target; global is C++20 in .bazelrc.

    gnu++26, not plain c++26: @system_libstdcxx//:std (every module target's own dep, added
    automatically by congelado_module_library) is itself compiled -std=gnu++26 — clang
    rejects loading a precompiled module whose GNU-extensions flag doesn't match the
    consuming TU's ("GNU extensions was enabled in precompiled file ... but is currently
    disabled" / -Wmodule-file-config-mismatch), so every consumer must match it exactly.
    """
    return congelado_copts() + ["-std=gnu++26"]

def congelado_cc_library(
        name,
        copts = congelado_cxx26_copts(),
        features = ["cpp_modules"],
        **kwargs):
    """Own cc_library; opts into C++26 named-modules machinery per-target.

    TF/xla_cc_binary replace semantics: passing copts/features REPLACES the
    defaults — merge via congelado_cxx26_copts() yourself if you add extras.
    """
    cc_library(
        name = name,
        copts = copts,
        features = features,
        **kwargs
    )

def congelado_cc_binary(
        name,
        copts = congelado_cxx26_copts(),
        features = ["cpp_modules"],
        **kwargs):
    """Own cc_binary; same defaults and replace semantics as congelado_cc_library."""
    cc_binary(
        name = name,
        copts = copts,
        features = features,
        **kwargs
    )

def congelado_module_library(
        name,
        primary_interface,
        partitions = [],
        hdrs = [],
        deps = [],
        visibility = None,
        copts = congelado_cxx26_copts(),
        features = ["cpp_modules"],
        alwayslink = False):
    """One Bazel target per C++ module; partitions are extra module_interfaces srcs, not separate targets.

    alwayslink: pass True when this module's only externally-visible effect is a static-init
    side effect nothing directly references (e.g. a self-registering factory, see
    include/cc/stable_hlo/BUILD) — otherwise the linker drops that translation unit's object
    from a static archive since nothing resolves a symbol out of it.
    """
    interfaces = [primary_interface] + partitions
    cc_library(
        name = name,
        module_interfaces = interfaces,
        hdrs = hdrs + interfaces,  # also as hdrs: sibling module sources must be real inputs, see memory
        copts = copts,
        linkopts = congelado_linkopts(),
        deps = deps + ["@system_libstdcxx//:std"],
        features = features,
        visibility = visibility,
        alwayslink = alwayslink,
    )

def congelado_cc_test(name, srcs, deps = [], **kwargs):
    """Wraps cc_test per apply_test_target() in xmake/common.lua — recompiles srcs fresh, never deps on the production target."""
    cc_test(
        name = name + "_test",
        srcs = srcs,
        copts = congelado_copts(),
        defines = ["CONGELADO_TEST"],
        deps = deps + ["//bazel/third_party:boost_ut"],
        **kwargs
    )

# check_deps: enforce structural invariants at build time (e.g. Phase 4d's engine static-lib rule).

CollectedDepsInfo = provider(
    doc = "Transitive set of dependency labels collected by collect_deps_aspect.",
    fields = ["labels"],
)

def _collect_deps_aspect_impl(target, ctx):
    labels = [str(target.label)]
    for dep in getattr(ctx.rule.attr, "deps", []):
        if CollectedDepsInfo in dep:
            labels += dep[CollectedDepsInfo].labels
    return [CollectedDepsInfo(labels = depset(labels).to_list())]

collect_deps_aspect = aspect(
    implementation = _collect_deps_aspect_impl,
    attr_aspects = ["deps"],
)

def _check_deps_impl(ctx):
    labels = depset(transitive = [
        depset(d[CollectedDepsInfo].labels)
        for d in ctx.attr.deps
    ]).to_list()
    for disallowed in ctx.attr.disallowed_deps:
        if str(disallowed.label) in labels:
            fail("congelado_check_deps: %s transitively depends on disallowed target %s" % (
                ctx.label,
                disallowed.label,
            ))
    return [DefaultInfo()]

congelado_check_deps = rule(
    implementation = _check_deps_impl,
    attrs = {
        "deps": attr.label_list(mandatory = True, aspects = [collect_deps_aspect]),
        "disallowed_deps": attr.label_list(default = []),
    },
)
