# Internal Plugins Architecture (`yoshi/omah_lay`)

This directory holds internal implementations of the C ABIs declared in `//include/c`.

---

## Core Architecture (Approach A: Per-Plugin Choice)

### 1. Minimal Rebuilds (Driven by `include/c`)
Every plugin in `omah_lay` depends on `//include/yoshi/omah_lay:plugin_c_abi_base`.
* **Action Cache**: Bazel content-hashes all header files in `include/c/`. Plugins are **only recompiled if their own source files change or headers in `include/c` change**.
* If changes happen in other areas of the repository, your plugins remain untouched and load instantly from the cache.

### 2. Optional GPU Stack (Zero Overhead for CPU Plugins)
Dependencies are declared strictly **per-plugin**:
* **Standard / CPU Plugins**: Depend only on `:plugin_c_abi_base`. Bazel will **never** fetch, build, or link SYCL or `libxFile`.
* **GPU / Direct Storage Plugins**: Explicitly add `:gpu_plugin_deps` (or `@adaptive_cpp//:sycl` and `//third_party/libxfile:libxfile`). Bazel will build the GPU toolchain **only** when building that specific GPU plugin.

---

## Usage Examples

### Standard Internal Plugin (CPU / Host Only)
```python
cc_binary(
    name = "custom_store_plugin.so",
    srcs = ["custom_store.cc"],
    linkshared = True,
    deps = [
        "//include/yoshi/omah_lay:plugin_c_abi_base",
    ],
)
```

### GPU-Accelerated Plugin (SYCL + Direct GPU-to-Disk Storage)
```python
cc_binary(
    name = "gpu_storage_plugin.so",
    srcs = ["gpu_storage.cc"],
    linkshared = True,
    deps = [
        "//include/yoshi/omah_lay:plugin_c_abi_base",
        "//include/yoshi/omah_lay:gpu_plugin_deps",
    ],
)
```
Building `custom_store_plugin.so` will never compile SYCL or libxFile, while building `gpu_storage_plugin.so` builds only what it requires!
