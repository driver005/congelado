# OpenXLA (XLA) — fetched via http_archive

XLA is fetched on-demand using a repository rule (`third_party/xla/repo.bzl`) via `http_archive`.

This mirrors the approach used for `reflect-cpp`, `stduuid`, and `system_libstdcxx`.

## Pin

| field  | value |
|--------|-------|
| commit | `e5d008bb03d9b133b0881daf3d108adea8e6625b` |
| date   | 2026-08-22 |

XLA has **no git tags or release branches** — consumers pin commits
(PyTorch/XLA does the same with its `xla_hash`; TensorFlow vendors this exact
way).

## Loading

Loaded via the `third_party_deps` module extension in `//third_party:extension.bzl`:

```python
load("//third_party/xla:repo.bzl", xla_repo = "repo")

def _third_party_impl(module_ctx):
    ...
    xla_repo()
```

The root `MODULE.bazel` declares:
```python
third_party_deps = use_extension("//third_party:extension.bzl", "third_party_deps")
use_repo(third_party_deps, "stduuid", "reflect_cpp", "system_libstdcxx", "xla")
```

## Module Extensions

XLA's module extensions are explicitly invoked in the root `MODULE.bazel`:
- `tsl_extension` — provides `@tsl` (with `bazel_issue_21519` workaround)
- `llvm_extension` — provides `@llvm-project`
- `third_party_ext` — provides `ml_dtypes_py`, `eigen_archive`, `net_zstd`, `stablehlo`, etc.
- `rocm_configure_ext` — provides `@local_config_rocm`

## Scope

C++ API only:

- `@xla//xla/client:client_library`
- `@xla//xla/service/cpu:cpu_plugin`

Never build/link `@xla//xla/python/...`, pybind11, or wheel targets.