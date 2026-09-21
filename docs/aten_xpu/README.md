# PyTorch XPU (SYCL) ATEN Reference Implementation

This directory contains a complete reference extraction of PyTorch's SYCL/XPU backend implementation from the ATEN (A Tensor Expression) library.

## Source
Extracted from: `pytorch/pytorch@main` (shallow clone)
- **Date**: 2026-09-18
- **Commit**: Latest main branch

## Directory Structure

```
aten_xpu/
├── core/                    # ATen/xpu/ - Core tensor creation & context
│   ├── EmptyTensor.{h,cpp}         # Tensor allocation (empty, empty_strided)
│   ├── XPUContext.{h,cpp}          # Device context management
│   ├── XPUEvent.{h,cpp}            # SYCL event wrapper
│   ├── XPUGeneratorImpl.{h,cpp}    # RNG state management
│   ├── XPUUtils.h                  # Utilities
│   ├── CachingHostAllocator.{h,cpp} # Host-pinned memory allocator
│   ├── MemPool.{h,cpp}             # Memory pool
│   ├── PhiloxXpuState.h            # RNG Philox state
│   ├── XPUDevice.h                 # Device abstraction
│   ├── XPUGraph.{h,cpp}            # CUDA Graph equivalent for XPU
│   ├── XPUGraphsUtils.h            # Graph utilities
│   ├── XPUScaledBlas.{h,cpp}       # BLAS scaling utilities
│   └── detail/
│       ├── LazyLevelZero.{h,cpp}   # Level Zero lazy init
│       ├── XPUHooks.{h,cpp}        # XPU hooks interface
│       └── level_zero_stub/        # Level Zero stubs
├── mkldnn/                  # ATen/native/mkldnn/xpu/ - oneDNN optimized kernels
│   ├── Attention.cpp              # Multi-head attention
│   ├── Blas.cpp                   # BLAS operations
│   ├── Conv.cpp                   # Convolution (forward/backward)
│   ├── Conv.h                     # Conv declarations
│   ├── Linear.cpp                 # Linear/GEMM
│   ├── RNN.cpp                    # RNN/LSTM/GRU
│   ├── ScaledBlas.cpp             # Scaled BLAS (FP8, BF16)
│   ├── FusionUtils.{h,cpp}        # Kernel fusion
│   ├── qconv.{h,cpp}              # Quantized convolution
│   ├── qlinear.{h,cpp}            # Quantized linear
│   └── detail/                    # oneDNN implementation details
│       ├── Attention.cpp
│       ├── Attr.h
│       ├── Conv.cpp
│       ├── Deconv.cpp
│       ├── DnnlExt.h
│       ├── LRUCache.h
│       ├── Matmul.cpp
│       ├── oneDNN.{h,cpp}
│       ├── oneDNNContext.{h,cpp}
│       ├── QConv.cpp
│       ├── QMatmul.cpp
│       ├── Utils.{h,cpp}
│       └── WoQMatmul.cpp
├── transformers/            # ATen/native/transformers/xpu/ - Transformer kernels
│   ├── attention.cpp              # Flash attention forward
│   ├── attention_backward.cpp     # Flash attention backward
│   ├── sdp_utils.{h,cpp}          # Scaled dot-product attention utils
├── c10/                     # c10/xpu/ - C10 level XPU support
│   ├── XPUCachingAllocator.{h,cpp} # Main device allocator (caching)
│   ├── XPUFunctions.{h,cpp}       # Device management functions
│   ├── XPUStream.{h,cpp}          # SYCL stream/queue wrapper
│   ├── XPUEvent.h                 # Event abstraction
│   ├── XPUDeviceProp.h            # Device properties
│   ├── XPUMacros.h                # XPU macros
│   ├── XPUException.h             # Exception handling
│   ├── XPUGraphsC10Utils.h        # Graph utilities
│   ├── PeerToPeerAccess.{h,cpp}   # P2P access
│   └── impl/                      # Implementation details
│       ├── XPUGuardImpl.{h,cpp}   # Device guard
├── torch_csrc/              # torch/csrc/xpu/ - Python bindings
│   ├── Module.{h,cpp}             # Python module
│   ├── Stream.{h,cpp}             # Stream Python bindings
│   ├── Event.{h,cpp}              # Event Python bindings
│   ├── Graph.{h,cpp}              # Graph Python bindings
│   ├── XPUPluggableAllocator.{h,cpp} # Pluggable allocator
│   ├── memory_snapshot.{h,cpp}    # Memory profiling
│   └── MemPool.{h,cpp}            # Memory pool Python bindings
├── async_support/           # NEW: ice::builder async integration layer
│   ├── xpu_async_tensor.cppm      # Async TF_TensorOps implementation
│   ├── xpu_async_buffer.cppm      # Async buffer allocation
│   ├── xpu_async_kernels.cppm     # Async kernel launchers
│   └── xpu_async.cppm             # Unified async API
└── xpu_dispatch_keys.txt    # All XPU: dispatch entries from native_functions.yaml
```

## Key Files for ice::builder Integration

### Core Tensor Operations
| File | Purpose |
|------|---------|
| `core/EmptyTensor.{h,cpp}` | `empty_xpu()`, `empty_strided_xpu()` - tensor allocation |
| `c10/XPUCachingAllocator.{h,cpp}` | `raw_alloc()`, `raw_delete()`, `emptyCache()` - memory management |
| `c10/XPUStream.{h,cpp}` | `XPUStream` wraps `sycl::queue` |
| `c10/XPUFunctions.{h,cpp}` | `device_count()`, `current_device()`, `get_raw_device()`, `get_device_context()` |

### Optimized Kernels (oneDNN)
| File | Operations |
|------|------------|
| `mkldnn/Attention.cpp` | Multi-head attention (Flash Attention style) |
| `mkldnn/Conv.cpp` | Convolution forward/backward |
| `mkldnn/Linear.cpp` | GEMM-based linear layers |
| `mkldnn/RNN.cpp` | LSTM/GRU/RNN |
| `mkldnn/ScaledBlas.cpp` | FP8/BF16 scaled matmul |

### Transformer Kernels
| File | Operations |
|------|------------|
| `transformers/attention.cpp` | SDPA forward |
| `transformers/attention_backward.cpp` | SDPA backward |
| `transformers/sdp_utils.{h,cpp}` | Attention masks, scaling |

## Dispatch Keys (from `xpu_dispatch_keys.txt`)

1178 operators registered with `XPU:` dispatch key including:
- Pointwise: `abs`, `add`, `mul`, `relu`, `gelu`, `silu`, etc.
- Reduction: `sum`, `mean`, `max`, `min`, `norm`
- Linear Algebra: `mm`, `bmm`, `addmm`, `linear`, `conv2d`, `conv3d`
- Attention: `scaled_dot_product_attention`, `flash_attention`
- RNN: `lstm`, `gru`, `rnn`
- Pooling: `max_pool2d`, `avg_pool2d`, `adaptive_pool2d`
- Normalization: `batch_norm`, `layer_norm`, `group_norm`
- Loss: `cross_entropy`, `mse_loss`, `nll_loss`

## Async Support Layer (ice::builder Integration)

### Files in `async_support/`
| File | Description |
|------|-------------|
| `xpu_async_tensor.cppm` | `XPU_AsyncTensorOps` - `TF_TensorOps` with SYCL async ops |
| `xpu_async_buffer.cppm` | `XPU_AsyncBuffer` / `XPU_PooledAsyncBuffer` - async alloc/copy |
| `xpu_async_kernels.cppm` | Async launchers for Attention, Conv, Linear, RNN |
| `xpu_async.cppm` | Unified API + factory functions |

### Key Features
- **SYCL Event-based async**: All operations return `sycl::event` for chaining
- **Pipeline support**: `XPU_AsyncPipeline` for multi-kernel dependency chains
- **Memory pools**: `XPU_PooledAsyncBuffer` for fast allocation from mempools
- **Stream management**: Full `c10::xpu::XPUStream` integration
- **C-ABI compatible**: Implements `TF_TensorOps` vtable for `ice::builder`

### Usage Example
```cpp
import cc_ice_builder_intern:xpu_async;

using namespace ice::builder;

// Create async tensor on XPU
auto tensor = make_xpu_tensor({{2, 3, 32, 32}, kFloat});

// Async copy from host
auto copy_result = tensor.copy_from_host_async(host_data, tensor.tensor_byte_size());
copy_result.wait();

// Launch attention kernel asynchronously
auto attention = make_xpu_attention();
auto attn_result = attention.forward_async(query, key, value, 0.1, true);

// Chain next operation
auto next_event = attn_result.event.then([&](auto) {
    // Next kernel...
});

// Or use pipeline
auto pipeline = make_xpu_pipeline();
pipeline.add([&]{ /* kernel 1 */ });
pipeline.add([&]{ /* kernel 2 */ }, &pipeline.last_event());
pipeline.wait_all();
```

## Building with Bazel

Add to your `BUILD.bazel`:
```python
cc_library(
    name = "xpu_async_impl",
    srcs = [
        "async_support/xpu_async_tensor.cppm",
        "async_support/xpu_async_buffer.cppm", 
        "async_support/xpu_async_kernels.cppm",
        "async_support/xpu_async.cppm",
    ],
    hdrs = glob([
        "core/*.h",
        "mkldnn/*.h",
        "mkldnn/detail/*.h",
        "transformers/*.h",
        "c10/*.h",
    ]),
    deps = [
        "@pytorch//aten:xpu",
        "@pytorch//c10:xpu",
        "//include/cc/ice/intern:builder_tensor",
        "//include/cc/ice/intern:builder_buffer",
        "//include/cc/ice/intern:builder_status",
    ],
)
```

## Registering with ice::builder C-ABI

In your plugin registration (e.g., `include/cc/ice/intern/registration.cppm`):
```cpp
TF_CAPI_EXPORT void create_tensor(TF_TensorOps** ops, void** ctx, TF_Status* status) {
    *ops = ice::builder::XPU_AsyncTensorOps::create();
    *ctx = *ops;
}

TF_CAPI_EXPORT void destroy_tensor(void* ctx) {
    ice::builder::XPU_AsyncTensorOps::destroy(static_cast<TF_TensorOps*>(ctx));
}
```

## SYCL Requirements
- **SYCL Implementation**: Intel oneAPI DPC++ / LLVM SYCL / hipSYCL
- **Level Zero**: Required for Intel GPU support
- **oneDNN**: Required for mkldnn kernels (DNNL_GRAPH=ON for graph API)
- **PyTorch XPU**: Build with `USE_XPU=ON` and `USE_SYCL=ON`

## Notes
- This is a **reference implementation only** — not a production build
- Kernels in `mkldnn/` and `transformers/` are oneDNN/SYCL implementations
- The async layer is designed for `ice::builder` C++20 modules architecture
- See `xpu_dispatch_keys.txt` for complete operator coverage