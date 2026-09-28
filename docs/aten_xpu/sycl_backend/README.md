# sycl_backend — reference SYCL implementation of the C layer

Reference implementation. `docs/` only, not built by Bazel or wired into `include/c/BUILD`.
Replaces `docs/aten_xpu/async_support/*.cppm` and the ATen-bound sources under
`docs/aten_xpu/{c10,core,mkldnn,transformers}` with a plugin built entirely from
`ice::builder::TF_*Ops` subclasses, following the C layer's flow instead of ATen's — see the
parent plan (`~/.claude/plans/please-look-at-the-encapsulated-squid.md`) for why.

## What is here

| File | Replaces | What it does |
|---|---|---|
| `device.cppm` | `c10/XPUDeviceProp.h`, `core/XPUContext.cpp` | Device properties, native handle |
| `platform.cppm` | `c10/XPUFunctions.cpp`, `c10/PeerToPeerAccess.cpp` | Device enumeration (dGPU first), shared context, current-device-per-thread, P2P |
| `stream.cppm` | (part of `c10/XPUStream.*`) | One in-order queue: priority, non-blocking query, sync, capture status |
| `event.cppm` | `c10/XPUEvent.h` | Timing, elapsed time |
| `timer.cppm` | (part of `async_support`) | Start/stop event pair |
| `executor.cppm` | `c10/XPUStream.cpp` | Stream pool, current stream, memcpy/memset, sync, event/timer lifecycle |
| `allocator.cppm` | `c10/XPUCachingAllocator.cpp` | Best-fit block allocator: split/coalesce, 512-byte alignment, stats, pools |
| `mem_pool.cppm` | `core/MemPool.{h,cpp}` | Named pool id + stream filter that the allocator reads |
| `memory.cppm` | (facade) | Owns one `SyclAllocator` per device |
| `tensor.cppm` | `core/EmptyTensor.cpp`, `async_support/xpu_async_tensor.cppm` | dtype/dims/strides/storage offset, bitcast, view, copy |
| `random_generator.cppm` | `core/XPUGeneratorImpl.{h,cpp}` | Philox seed/offset/state |
| `philox_codec.h` | `core/PhiloxXpuState.h` | Portable, unchanged math |
| `device_graph.cppm` | `core/XPUGraph.{h,cpp}` | `command_graph` capture/instantiate/replay |
| `optimizer.cppm` | — | Graph-level fusion pass (`TFGrapplerOptimizerOps::optimize`): parses/rewrites the real `tensorflow::GraphDef` (`include/cc/proto/{graph,node_def,attr_value}.proto`), collapsing Conv2D/Addmm/Bmm+BiasAdd(+Relu) into one node, feeding `kernels/matmul.cppm` / `conv.cppm`'s "activation" post-op |
| `kernels/engine_cache.cppm` | `mkldnn/detail/oneDNNContext.h` | Per-device `dnnl::engine` / per-queue `dnnl::stream` cache |
| `kernels/kernel_context.h`, `kernel_construction_view.h` | — | Hand-written views over the host-implemented `TF_OpKernelContext(Construction)Ops`; see below |
| `kernels/matmul.cppm` | `mkldnn/detail/Matmul.cpp` | addmm / bmm |
| `kernels/conv.cppm` | `mkldnn/detail/Conv.cpp` | 2D convolution, forward only |
| `kernels/linear.cppm` | `mkldnn/Linear.cpp` | `x @ weight^T + bias` |
| `kernels/sdpa.cppm` | `mkldnn/detail/Attention.cpp`, `transformers/attention.cpp` | Naive (non-fused) attention — see the file's header comment |
| `kernels/sdpa_backward.cppm` | `transformers/attention_backward.cpp` | Backward for the same naive attention, recomputes the forward softmax |
| `kernels/deconv.cppm` | `mkldnn/detail/Deconv.cpp` | 2D transposed convolution, forward only, ungrouped |
| `kernels/int8_conv.cppm` | `mkldnn/detail/QConv.cpp`, `mkldnn/qconv.*` | Per-tensor int8 quantized 2D convolution |
| `kernels/int8_matmul.cppm` | `mkldnn/detail/QMatmul.cpp`, `mkldnn/qlinear.*` | Per-tensor int8 quantized matmul |
| `kernels/woq_matmul.cppm` | `mkldnn/detail/WoQMatmul.cpp` | Weight-only int4 (GPTQ-style, grouped scale/zero-point) matmul |
| `kernels/scaled_mm.cppm` | `mkldnn/ScaledBlas.cpp`, `core/XPUScaledBlas.*` | Tensor-wise scaled matmul (`ScalingType::TensorWise` only) |
| `kernels/rnn_lstm.cppm` | `mkldnn/RNN.cpp` | Single-layer, single-direction LSTM forward (inference) |
| `plugin.cc` | `docs/aten_xpu/torch_csrc/Module.cpp` | Entry points only |

Deleted, superseded by the above: `async_support/`, `torch_csrc/*` (Python bindings),
`c10/impl/XPUGuardImpl.*`, `core/detail/LazyLevelZero.*`, `level_zero_stub/`,
`core/detail/XPUHooks.*`.

Not converted: `mkldnn/detail/{Attr,FusionUtils,DnnlExt,LRUCache,Utils}.*` (`Attr.h`'s fuller
post-op set beyond the one ReLU fusion above, and primitive-cache helpers — `EngineCache` builds
a fresh `dnnl::matmul`/`dnnl::convolution_forward` per call rather than caching the primitive
itself), the `c10/test/` tree.

## Assumptions this code makes about the still-moving C-layer generator

1. **`create_*_internal` factory slots take a raw handle, not a dereferenced reference.** The
   generator currently emits `create_device_internal(const ice::sonic::Device& device)` and then
   calls it as `self->create_device_internal(*ice::builder::Device::create(device))` —
   dereferencing `device->plugin_data` before anything has set it. Every `create_*_internal`
   slot in this plugin (device, executor, stream, event, timer, random generator, allocator, mem
   pool) is written as if it took `TF_X*` instead, so the plugin can assign `plugin_data` itself.
2. **Kernels do not use the generated `ice::sonic::OpKernelContext`.** Its `Runtime` base always
   constructs its own internal handle value; it has no way to wrap the externally-supplied
   `TF_OpKernelContext*` a kernel's `compute_func` actually receives. `kernels/kernel_context.h`
   and `kernel_construction_view.h` call the host-implemented ops tables directly instead — the
   same thing a real TF C-API plugin kernel does.
3. Every cross-vtable *input* reference (an already-created handle passed into a slot) is typed
   `ice::builder::X&`, matching what the generated vtable lambdas actually construct
   (`*ice::builder::X::create(handle)`), not what the currently-generated pure-virtual
   declaration says (`ice::sonic::X&` in most places right now).

If the generator's Builder-tier signatures change to match, only the override signatures in
this tree need to follow — the logic inside each method does not depend on the assumption.

## Known gaps (each flagged in its own file too)

- `random_generator.cppm`: `set_state`/`get_state`/`graphsafe_*` need `SyclTensor` for the
  extragraph seed/offset tensors — not wired up, returns "not implemented".
- `random_generator.cppm`: `clone()` allocates with raw `new`, untracked by any executor —
  `TF_RandomGeneratorOps` has no `destroy` slot of its own to free it.
- `allocator.cppm`: no expandable segments, no IPC, no per-stream pool partitioning (best-fit
  search does not prefer blocks last used on the same stream).
- `executor.cppm`: `get_stream_status` and `memory.cppm`'s `enable_peer_access` are no-ops — a
  real backend needs an async-error handler on the queue and
  `ext_oneapi_enable_peer_access` respectively.
- `kernels/matmul.cppm`, `conv.cppm` have one fused post-op (ReLU, via `optimizer.cppm`'s graph
  rewrite). `linear.cppm`, `deconv.cppm` have none. None of the four has `Attr.h`'s fuller
  post-op set (arbitrary binary ops, more eltwise kinds) or a backward pass.
- `optimizer.cppm` does not consult `TFGrapplerItem`'s nodes-to-preserve / fetch-node lists —
  a node that must survive unmodified could still be fused away.
- `kernels/sdpa.cppm` / `sdpa_backward.cppm`: naive three-primitive attention, not the real
  fused/tiled flash-attention algorithm — the external `sycltla::` library
  `transformers/attention.cpp` calls is not vendored in this tree. Backward recomputes the
  forward softmax rather than taking it as a saved input.
- `kernels/int8_conv.cppm`, `int8_matmul.cppm`: per-tensor quantization only — no per-channel
  weight scale, no binary/unary post-ops, no bias broadcasting
  (`broadcast_bias2D`/`broadcast_bias3D`).
- `kernels/scaled_mm.cppm`: `ScalingType::TensorWise` only — RowWise and the four BlockWise
  (fp8 microscaling) recipes `core/XPUScaledBlas.cpp` supports are not ported; reads operands as
  s8, not f8_e4m3/f8_e5m2.
- `kernels/rnn_lstm.cppm`: single layer, single direction — no bidirectional concat, no
  multi-layer loop, no PyTorch flat-params-list unpacking (the caller must already hand in
  per-layer-shaped weights).
- `plugin.cc`: which plugin-entry convention a real host calls (`create_plugin` vs `init_plugin`
  vs `congelado_init`) is unresolved; this file implements `create_plugin` since
  `torch_csrc/Module.h` already assumes it.

## Verification (once the C layer / generator settle)

- Build against AdaptiveCpp; smoke test: allocate → memcpy → addmm via the registered kernel →
  compare to a CPU reference.
- Grep gate on every file here: no `at::`, `c10::`, `TORCH_`, `thread_local` (the two documented
  per-calling-thread exceptions in `platform.cppm`/`executor.cppm` aside); `sycl::`/`dnnl::` only
  inside this tree.
