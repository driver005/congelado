# aten_xpu: SYCL backend on the ice layer

Reference SYCL/oneDNN plugin that implements the generated `ice::builder::*` interfaces.
It lives under `docs/` and Bazel does not build it.

## Layout

The tree mirrors `include/cc/ice`. Each directory is one C++ module, and each file is one partition holding one class.

```
docs/aten_xpu/
├── base.cppm                 aten_xpu (re-exports intern + extern)
├── plugin/                   one C entry file per plugin library (create_plugin, create_*/destroy_*)
├── plugin_entry.h            export macros shared by the plugin entry files
├── intern/                   aten_xpu_intern
│   ops_table, handle, status, tensor, tensor_factory, and the plain C++ builders
│   (no SYCL): string, status_store, buffer, registration, complex, duration, time_point,
│   data_type, attr_type, shape, hash, file_statistics, text_tensor, array, vector, deque,
│   bit_set, span, list, forward_list, hive, map, set, failure
├── extern/                   aten_xpu_extern
│   ├── stream_executor/      platform, device, device_guard, peer_access, level_zero,
│   │                         executor, stream, stream_pool, event, timer, memory, mem_pool,
│   │                         stream_executor, allocator/{allocator, block, block_pool,
│   │                         expandable_segment, stats, trace, snapshot, ipc_memory,
│   │                         host_cache, pluggable_allocator}
│   ├── random_generator/     random_generator, philox_state, philox_codec
│   ├── grappler/             grappler, device_graph, capture_status, optimizer, configs, pipeline
│   │                         (graph_index, graph_editor, const_tensor, properties, op_lookup, and
│   │                         the passes: pruning, constant_folding, arithmetic, dependency,
│   │                         batch_norm_folding, shape, function_inlining,
│   │                         dead_branch, debug_stripper, pin_to_host, mixed_precision,
│   │                         remapper)
│   ├── filesystem/           filesystem, random_access_file, writable_file,
│   │                         read_only_memory_region, tree (std::filesystem + POSIX)
│   ├── kernel/               kernel (SyclKernel<Derived> : ice::builder::Kernel), registrar,
│   │                         context/, onednn/, matmul/, linear/, convolution/,
│   │                         quantized/, scaled/, attention/, recurrent/, ops/ (op definitions
│   │                         and shape inference)
│   └── registration/         SyclPluginRegistry (all vtables, host ops, op and kernel registration)
└── test/                     gtest suites on the ice::sonic wrappers (fixture.cppm)
```

## Lifecycle

- The host calls a vtable's `create` slot. The plugin allocates the concrete `Sycl*` object and `create` stores `plugin_data` from the new object in `plugin_data`. `destroy` deletes the object.
- `create_*_internal(parent, handle)` binds an object that already exists (for example `SyclStream::bind`). Generated wrappers copy the `{plugin_data}` handle, so `T::from_handle` finds the same object.
- Pool and current-stream slots (`get_stream_from_pool`, `get_current_stream`, `get_default_random_generator`) bind the caller's handle to shared state, such as a shared queue or a shared Philox state. They never swap `plugin_data`.
- `SyclOpsTable` holds two kinds of ops tables: host tables (status, string, buffer, kernel context/construction, shape context and handle, grappler item, function library and properties) and the plugin's own vtables. `SyclPluginRegistry::initialize` fills it.

## Generated API conventions

- Class names drop `TF_` and `Ops`: `ice::builder::Device`, `ice::sonic::Registration`.
- Plain C structs and enums have value classes in `ice::sonic` (`PoolId`, `DeviceProperties`, `MemorySpace`, `Code`, ...). Struct parameters are `ice::sonic::T*` (NULL stays possible); enums go by value or as `X&` for `out_*`.
- Scalar `out_*` parameters are references (`std::int32_t& out_x`).
- Arrays: the implementer (`ice::builder`) gets `std::span` over the host's array, the caller (`ice::sonic`) passes a `std::vector`.
- Nothing in the plugin is `static` except the two `getInstance()` entry points; `create` slots are lambdas in `SyclPluginRegistry`.

## Legacy mapping

| Legacy | Now |
|---|---|
| `c10/XPUStream.*`, `torch_csrc/Stream.*` | `stream`, `stream_pool`, executor current stream |
| `c10/XPUEvent.h`, `torch_csrc/Event.*` | `event` (timing and IPC) |
| `c10/XPUFunctions.*`, `c10/PeerToPeerAccess.*`, `core/XPUDevice.h` | `platform`, `peer_access` |
| `c10/XPUDeviceProp.h`, `core/XPUContext.*` | `device` |
| `c10/impl/XPUGuardImpl.*` | `device_guard` |
| `core/detail/LazyLevelZero.*` | `level_zero` |
| `c10/XPUCachingAllocator.*`, `torch_csrc/memory_snapshot.*` | `allocator/` |
| `core/CachingHostAllocator.*` | `allocator/host_cache` (`TF_MEMORY_SPACE_HOST_PINNED`) |
| `torch_csrc/XPUPluggableAllocator.*` | `allocator/pluggable_allocator` |
| `core/MemPool.*` | `mem_pool` |
| `core/XPUGeneratorImpl.*`, `PhiloxXpuState.h` | `random_generator/` |
| `core/XPUGraph.*`, graph utils | `grappler/device_graph`, `capture_status` |
| `mkldnn/**`, `transformers/**`, `core/XPUScaledBlas.*` | `kernel/**` |
| `core/detail/XPUHooks.*`, `torch_csrc/Module.*` | `registration`, `plugin/*.cc` |
| `torch_csrc/profiler/*` (XPU activity trace) | `extern/profiler`: SYCL event profiling, XSpace output |
| `c10/test/**` | `test/` |

The Python bindings and CMake glue were dropped. Every behavior they exposed is now reachable through the ice slots.

## Profilers

Four independent `ice::builder::Profiler` implementations, each its own plugin library with its own `create_profiler` (one per library is what the C ABI allows). The three vendor ones do not use SYCL.

| Profiler | Library | Reports | Needs |
|---|---|---|---|
| SYCL (`extern/profiler/sycl`) | the main `aten_xpu` plugin | what the plugin submits: copies, fills, oneDNN primitives, host callbacks, allocator actions | `aspect::queue_profiling`; `ATEN_XPU_PROFILING=1` at load for device timestamps |
| PTI (`extern/profiler/pti`) | `libaten_xpu_profiler_pti.so` | every kernel and copy on an Intel GPU, real kernel names | Intel PTI (`libpti_view.so`) |
| CUPTI (`extern/profiler/cupti`) | `libaten_xpu_profiler_cupti.so` | every kernel, copy and fill on an NVIDIA GPU | CUPTI (`libcupti.so`) |
| rocprofiler (`extern/profiler/rocprofiler`) | `libaten_xpu_profiler_rocprofiler.so` | every kernel dispatch and copy on an AMD GPU | rocprofiler-sdk |

All of them return a TensorFlow `XSpace` (protobuf, `extern/profiler/proto/xplane.proto`, vendored from openxla/xla) as a `uint8` tensor.

- **SYCL:** works on every device with `aspect::queue_profiling` (Level Zero, OpenCL, CUDA, HIP). Without `ATEN_XPU_PROFILING=1` queues have no profiling, so `start` still works but activities show at submission time with zero duration and the XSpace carries a warning. One XPlane per device, one XLine per queue and kind, plus an `allocator` line. Not recorded: copies on throwaway queues (`blocking_copy`, tensor copies, host reads), the Philox graph-capture fills and barriers.
- **Vendor profilers:** `common/` holds what they share (tracer interface, `VendorProfiler`, a host-memory tensor, the plugin owner). Each library also exports `create_tensor`, because the host needs the tensor ops to read the returned `XSpace`. One XPlane per device the tracer reported (`/device:<tracer>:<device>`), one XLine per queue and kind. `start` fails with a status error if the tracer is missing.
- Each tracer is optional twice over: its header (`pti/pti_view.h`, `cupti.h`, `rocprofiler-sdk/rocprofiler.h`) at build time, its library at run time.
- PTI and CUPTI generally have to start before the GPU runtime initialises. rocprofiler-sdk only reports if it loads its library before HSA starts (it finds the exported `rocprofiler_configure`).
- The CUPTI record structs follow the header (`CUpti_ActivityKernel9`, `Memcpy6`, `Memset4`, CUPTI 12.9); another CUPTI version may need other struct names.
- Build the vendor libraries with `--force_pic`: `bazel build --force_pic //docs/aten_xpu/extern/profiler/pti:libaten_xpu_profiler_pti.so` (protobuf is linked statically).

## Kernels

| Op | Class |
|---|---|
| Addmm, BatchMatMul, Baddbmm, Addmv | `matmul/*` |
| Linear (unary/binary fusion) | `linear/linear` |
| Conv2D and Conv3D (one N-d kernel, groups), ConvolutionBackward, transposed conv | `convolution/*` |
| QuantizedMatMul and QuantizedLinear (`weight_transposed`), QuantizedConv2D | `quantized/int8_*` |
| WeightOnlyQuantizedMatMul (u4, grouped) | `quantized/woq_matmul` |
| ScaledMatMul (tensor-wise, row-wise, block-wise recipes) | `scaled/*` |
| ScaledDotProductAttention, its backward | `attention/*` (oneDNN graph fused path plus math path) |
| LSTM (multi-layer, bidirectional, inference) | `recurrent/lstm` |

## Plain C++ builders

Every `intern/` builder and the filesystem family have a plain C++ implementation (`String`, `Status`, `Vector`, `FilesystemTree`, ...). None of them needs SYCL.

- `SyclPluginRegistry` owns one prototype of each class, so every C vtable has a stable address, and getters such as `getVectorOps()` return it. There is no second registry.
- Each file in `plugin/` exports `create_<name>` for its own domain (`intern`, `stream_executor`, `random_generator`, `grappler`, `kernel`, `filesystem`, `profiler`, `gradients`); every domain builds as its own `libaten_xpu_<domain>.so` and brings up only its part of the registry. `create_string`, `create_status` and `create_buffer` are weak, so a real host definition overrides them; the other types bind to whichever status and string ops are in effect.
- Values the C ABI cannot carry come from the constructor: `Map` and `Set` take key and value sizes, `BitSet` its size, `Span::bind` its memory, `Duration` and `TimePoint` their ticks, and the files and memory region their path. Objects made through a `create` slot use defaults (8 byte keys and values, 64 bits, no path).
- Elements of the containers are byte erased: call `set_element_size` first. Pointers handed out by `get`, `find` and `data` are valid until the container changes.
- `FilesystemTree` returns string tensors as `TextTensor`: one dimension, every string NUL terminated back to back. The caller frees them with the tensor's `delete_tensor`.
- Tests: `test/{foundation,value,sequence,associative,filesystem}_test.cc` use `RegistryFixture` and need no XPU device.

## Op definitions

`SyclPluginRegistry::initialize` registers the ops before the kernels: `SyclOpCatalog` lists them, `SyclOpRegistrar` calls the host's `create_op_definition_builder` for each, and `SyclShapeInference` supplies the shape function (a `SyclShapeRule` template argument, because the C callback carries no context).

- The ops layer is host provided, like the kernel context: the plugin calls `create_op_definition_builder`, `create_shape_inference_context` and `create_shape_handle`, and exports none of them. This is read from the header shapes, not confirmed against a host.
- Defined: `Addmm`, `Addmv`, `Baddbmm`, `Linear`, `BatchMatMul`, `Conv2D`, `Conv3D`, `ConvolutionBackward`, `Conv2DBackpropInputTransposed`, `QuantizedMatMul`, `QuantizedLinear`, `QuantizedConv2D`, `WeightOnlyQuantizedMatMul`, `ScaledDotProductAttention`, `ScaledDotProductAttentionGrad`, `LSTM`, `ScaledMatMul`, `Transpose`, `_CopyFromXpuToHost`, `_CopyFromHostToXpu`, `_ScopedAllocator`, `_ScopedAllocatorConcat` and `_ScopedAllocatorSplit`. Each op has the name its kernel registers with and the attrs that kernel reads (for example `Conv2D` takes `stride`, `padding` and `dilation` as int lists, `data_format` (`NCHW`, or `NCDHW` for `Conv3D`; any other value fails at construction) and `activation`, which the grappler fusion sets). The scoped allocator ops carry the group's shapes as `total`, `ranks` and `dims` int attrs, not as TensorFlow's `shape` attrs.
- The spec strings (`T: {float, half}`, `list(type) >= 0 = []`, `N * T`) follow TensorFlow's op definition grammar, which the host's op definition builder has to parse.
- Optional inputs are a trailing `list(type)` input (`extras`, `scales`, `weights`), so the kernels' `getInputCount()` checks still work.
- The shape context has no int or list attr getter (`get_attr_type` is the only one), so some shapes stay partial. Every op has a shape rule. Exact: the matmul family, `Linear`, `WeightOnlyQuantizedMatMul`, `ScaledMatMul`, attention, both gradients and the `LSTM` states.
- Partly known, because the shape context can only read a dtype attr, not the int attrs: convolution and `QuantizedConv2D` give `[batch, out_channels, ?...]` (stride, padding and dilation set the spatial extents), `Conv2DBackpropInputTransposed` gives `[batch, ?...]` (channels are `weight[1] * groups`), and `QuantizedLinear` and `QuantizedMatMul` give `[..., ?]` (`weight_transposed` picks the column axis). The `LSTM` output and the attention probabilities stay unknown.

## Grappler passes

`SyclGrapplerOptimizer` runs `SyclGraphPipeline`, which applies the plugin's passes in the order of TensorFlow's meta optimizer. The host's function library and properties ops give the passes the op definitions (`is_stateful`, attrs, inputs) and the shapes (from the shape functions in `SyclOpCatalog`).

| TensorFlow optimizer | Plugin pass |
| --- | --- |
| Function | `SyclFunctionInliningPass`: inlines calls (a direct call or `PartitionedCall`) to functions of up to 128 body nodes without control returns, recursion or a call with control edges, nested calls included; it needs the host's op definitions to place each returned value on an output port |
| Pruning | `SyclPruningPass`: drops nodes the fetch and preserved nodes do not reach, unless the host defines the op as stateful; an op the host does not define stays |
| Constant folding | `SyclConstantFoldingPass`: float32 elementwise ops (`Add`, `Sub`, `Mul`, `RealDiv`, `Maximum`, `Minimum`, `Neg`, `Abs`, `Square`, `Sqrt`, `Rsqrt`, `Reciprocal`, `Exp`, `Log`, `Relu`, `Tanh`, `Sigmoid`, `Identity`) over `Const` inputs of one shape or scalars, up to 2^20 elements |
| Arithmetic | `SyclArithmeticPass`: `x * 1`, `x + 0`, `x - 0`, `x / 1` against scalar constants, `Neg(Neg(x))`, `Reciprocal(Reciprocal(x))` |
| Dependency | `SyclDependencyPass`: removes `Identity` nodes (not next to control flow ops or across devices), dead `NoOp` nodes, duplicate control inputs and control inputs that repeat a data input |
| Shape | `SyclShapePass`: `Shape`, `Size` and `Rank` of an input whose shape the host's inference knows completely become `Const` nodes (`out_type` int32 or int64) |
| Loop: dead branches | `SyclDeadBranchPass`: a `Switch` on a `Const` bool leaves one live output; the dead region is removed and a `Merge` left with one live input is bypassed. A while loop whose `LoopCond` compares constants (`Less`, `LessEqual`, `Greater`, `GreaterEqual`, `Equal`, `NotEqual`) and is false on entry never runs: its body goes and the loop variables keep their entering values. A region that reaches a preserved node or a loop op (other than `NextIteration` of a loop that never runs) is left alone |
| Loop: invariant motion | `SyclLoopInvariantMotionPass`: in an outermost loop frame, a stateless op (the host defines `is_stateful` false) whose inputs are all constant `Enter` nodes, `Const` nodes or other invariant nodes reads the pre-loop value instead, and the loop reads its result through a new constant `Enter` named `loop_invariant/<node>_<port>`. Nodes a variant node waits on through a control edge stay; nested loops are left alone |
| Loop: stack push removal | `SyclStackPushRemovalPass`: when no pop of a `Stack`/`StackV2` is read (and the stack and pops are not preserved), each push becomes an `Identity` of the pushed value with the stack handle as a control input |
| Layout | `SyclLayoutPass`: converts channels-last `Conv2D`, `Conv3D`, `BiasAdd` and `FusedBatchNorm*` nodes (`data_format` `NHWC` or `NDHWC`) to `NCHW` or `NCDHW` with `Transpose` nodes (shared per tensor) at the borders; elementwise nodes that only read converted nodes and scalars follow. Needs the rank and dtype from the host's inference |
| Memory: swapping | `SyclSwappingPass`: when the estimated peak device memory (`SyclGraphMemory`: topological order, node time from bytes over the device's memory bandwidth) exceeds the budget (`memory_limit_bytes`, default the device's global memory), tensors idle at the peak move to the host for their late readers: `swap_out_<reader>_<slot>` (`_CopyFromXpuToHost`) and `swap_in_<reader>_<slot>` (`_CopyFromHostToXpu`), over 1 KiB, ranked by idle time and age, with a control trigger on `swap_in` so it ends just before the reader |
| Memory: recomputation | `SyclRecomputationPass` (only when `memory_optimization` is switched on): cheap ops (`Add`, `Mul`, `Relu`, `Sigmoid`, `Tanh`, `Cast`, ... or a `_recompute_hint` attr) read by nodes in the `gradients/` scope are copied as `Recomputed/<node>` behind a `RecomputeTrigger` `NoOp` |
| Auto parallel | `SyclAutoParallelPass` (off by default): replicates the training fan-in per replica (`AutoParallel-Replica-<i>/`, `/device:XPU:<i mod devices>`), shares variables and the input pipeline in front of the first dequeue, divides gradients of the `Apply*` ops by the replica count and gates the fetches with `AutoParallel-Control-Fetch`; replica count from `auto_parallel_replicas` |
| Scoped allocator | `SyclScopedAllocatorPass` (off by default): groups ops of the configured kinds (`scoped_allocator_enable_ops`, default `CollectiveReduce`) in one name scope, device and dtype into `_ScopedAllocator`, `_ScopedAllocatorConcat`, one op over the buffer and `_ScopedAllocatorSplit` |
| Debug stripper | `SyclDebugStripperPass` (off by default): removes `Assert` and `PrintV2`, passes `Print` through, turns `CheckNumerics` into `Identity` |
| Pin to host | `SyclPinToHostPass` (off by default): moves `Shape`, `Size`, `Rank` and the small int32 ops and `Const` nodes around them to `/device:CPU:0` |
| Auto mixed precision | `SyclMixedPrecisionPass` (off by default): runs `Conv2D`, `Conv3D`, `Linear`, `Addmm`, `Addmv`, `Baddbmm` and `BatchMatMul` in half (bfloat16 when `auto_mixed_precision_onednn_bfloat16` is on), with `Relu`, `Relu6`, `Sigmoid`, `Tanh` and `Identity` following them, and casts at the borders (shared per tensor); preserved nodes stay float32 |
| Remapper | `SyclRemapper`, below |
| (inside TensorFlow's remapper) | `SyclBatchNormFoldingPass`: folds an inference `FusedBatchNorm` into the weights of the `Conv2D` or `Conv3D` before it, when the weights and statistics are float `Const` nodes; the batch norm becomes a `BiasAdd` the remapper then fuses |

Every pass obeys the host's optimizer settings, which the host sets through `SyclGrapplerConfigs`: `constant_folding`, `arithmetic_optimization`, `dependency_optimization`, `shape_optimization`, `function_optimization`, `loop_optimization`, `layout_optimizer`, `memory_optimization` (swapping) and `disable_model_pruning` run unless switched off; `debug_stripper`, `pin_to_host_optimization`, `auto_mixed_precision`, `auto_parallel`, `scoped_allocator_optimization` and recomputation (`memory_optimization` on) run only when switched on, as in TensorFlow. The remapper always runs. Three fields at the end of `TFGrapplerOptimizerConfigs` carry what TensorFlow keeps in its rewriter config: `auto_parallel_replicas` (0 means 1), `memory_limit_bytes` (0 means the device's global memory) and `scoped_allocator_enable_ops` (comma separated, `NULL` means `CollectiveReduce`).

Differences from TensorFlow, and what is unverified:

- The memory estimate is static: there is no cost graph or virtual cluster, so node time comes from bytes over memory bandwidth, and swapping assumes the 16 GB/s TensorFlow assumes for the link. A swap-in trigger is a node the reader already depends on; TensorFlow also adds a swap-out trigger.
- The gradient input positions of the `Apply*` ops in auto parallel are TensorFlow's table written from memory.
- The plugin's `Transpose`, host copy and scoped allocator kernels (`extern/kernel/transpose`, `copy`, `scoped`) are SYCL code that cannot be compiled without a SYCL toolchain; `_ScopedAllocatorConcat` and `_ScopedAllocatorSplit` copy, because the host's executor, not the plugin, decides where producers write. The graph passes are tested against a fake host.
- Loop invariant motion handles outermost frames only (TensorFlow also adds a control dependency on the parent loop for constants in nested frames).

The host still runs its own built-in versions of every optimizer; `SyclGrapplerConfigs` only turns its remapper off, so the plugin's passes run in addition to them.

The passes read the properties of the input graph once. Nodes that an earlier pass rewrites keep their input positions and shapes, so the later passes' queries stay valid; a pass that changed shapes would have to invalidate them first.

### Remapper

`SyclRemapper` follows TensorFlow's remapper (`core/grappler/optimizers/remapper.cc`) and uses the ops layer to decide what it may fuse:

- `SyclGraphIndex` indexes the GraphDef (name lookup, per-port data fanout, control edges, topological order), so matching is linear and not a scan per node.
- `SyclRemapper` walks the graph in reverse topological order and matches from the outermost op: `activation <- BiasAdd <- contraction`, `BiasAdd <- contraction` and `activation <- contraction`. An intermediate must have exactly one data consumer, no control edges, and not be a node the item preserves. The root keeps its name, so its consumers need no change.
- `SyclGraphProperties` runs the host's static shape inference (`infer_statically`, which calls the shape functions registered by `SyclOpCatalog`) and reads the input shapes: the bias must be a vector as long as the contraction's output channels.
- `SyclOpLookup` reads the op definition from the host (`look_up_op_def`) and fuses only when the op declares the attrs and inputs the rewrite sets (`Textras`/`extras` for the bias, `unary` or `activation` for the activation).
- Fusable: `Conv2D`, `Conv3D` and `Linear` take the bias as the first `extras` input and the activation as `unary` (`Relu`, `Relu6`, `LeakyRelu`, `Sigmoid`, `Tanh`, `Gelu`, `Swish`); `Addmm`, `Addmv`, `Baddbmm` and `BatchMatMul` take the activation as the `activation` code (`Relu`, `Sigmoid`, `Tanh`, `Swish`, exact `Gelu`). Nodes that already carry a bias or an activation are left alone.
- Checks mirror TensorFlow's: `T` is float, half or bfloat16, the node is on an XPU device (or unplaced), both nodes have the same dtype, and a `Conv2D` bias uses `data_format = NCHW` (a `Linear` bias `NHWC`).
- The properties handle is bound to the grappler item and the function library handle to the graph buffer (`bind` slots at the end of `TFGrapplerPropertiesOps` and `TFGrapplerFunctionLibraryOps`). If the host does not serve them or a bind fails, the remapper falls back to the structural checks and the passes that need them do nothing.
- `tensor_properties.proto` is wire compatible with `OpInfo.TensorProperties`, which is what the properties ops return.
- `Add` and `AddV2` count as a bias add when the host's shape inference proves it, with either input as the contraction: for `Linear` the other input is a vector as long as the output features, for `Conv2D` and `Conv3D` it has shape `[1, C, 1, ...]` (the convolution reads its bias by element count, so no reshape is needed).
- `SyclGrapplerConfigs` (exported as `create_grappler_configs`) tells the host to turn its own remapper off, because it would rewrite the same chains into fused ops this plugin has no kernels for. Every other built-in optimizer (constant folding, arithmetic, layout, memory, ...) keeps running in the host with its default setting; the plugin does not reimplement them.
- Not done in the remapper: a fused op for batch norm statistics that are not constant, and invalidating properties after a rewrite (it only queries nodes of the input graph).

## Known gaps



- ABI slots the plain C++ builders had to guess, to be fixed in the C headers: `Map` and `Set` have no key or value size, `Span` and `BitSet` have no bind or size slot, `Duration` has no setter (`TimePoint` fills only a `Duration`), `FilesystemTree::paths_exist` receives one String for an array (only the first path is checked), `free_options` has no context, and the filesystem has no open slot.

- LSTM runs forward inference only; no training workspace.
- SDPA has no dropout. The fused path covers non-causal attention; causal attention uses the math path.
- The math path supports grouped-query attention, except with an explicit mask or in backward.
- `create_kernel` is not exported; kernels register through `SyclKernelRegistrar`.
- Not compiled anywhere yet. This needs a SYCL compiler (icpx/AdaptiveCpp), oneDNN, and Level Zero headers.
