# ABI Code Style Principles — `include/c` + `include/cc/abi`

Status: ✅ ALL principles applied (2026-08-29). Only known remaining deviations are documented
pre-existing issues (api.h `unsigned char` legacy, api_experimental dead `TFE_*`, safe_ptr/
  tensor_interface legacy headers) and the registration.h char-keyed-registry exception.
Newest rules (2026-08-29 round 2): P7-Naming and P18 below.

The one style question that matters: **every C and C++ ABI surface follows the same principles**.
This document states each principle, why it exists, and the current conformance status with
evidence. Status key: ✅ conformant · ⚠️ partial · ❌ violations exist (file:line).

---

## 1. C ABI tier — `c/abi`, `c/intern`, `c/extern`

### P1 — `TF_Status*` is the only error channel
Any operation that can fail reports errors through a `TF_Status*` — always the **last**
parameter. Slots return a value (handle/int/bool-answer) or `void`; never a bool success flag,
never a sentinel-only convention. Pure allocation functions are the one exception: they may
return a handle or NULL with no status (the allocation itself is the failure signal).
- ✅ Conformant: `generator.h` bool slots → void (done); remaining bool slots are value-returns
  (`is_directory`, `attribute__is_list`, `typeinfo__is_*`).
- ⚠️ The allocator exception is not stated anywhere — codify it.

### P2 — Objects cross the ABI as pointers; never return them by value
✅ fixed: `TF_GetOpList` → `TF_Buffer*` (api.h); `TF_GetBuffer` free fn → `const TF_Buffer_Data*` non-owning view pointer. (Value views like `TF_Buffer_Data` remain by-value — documented view exception.)
(everywhere else handles are pointers — `TF_GetAllOpList` → `TF_Buffer*`.)

### P3 — Ownership is explicit
Every creator documents the matching destroyer; destroyers are null-safe.
- ✅ `get_definitions`, `parameter__get_type` documented (done).
- ❌ `filesystem.h` tensor-carrier returns (`get_children`, `get_matching_paths`,
  `get_filesystem_configuration`, `get_filesystem_configuration_keys`) document no destroyer —
  the doc comment says "freed with free_options" for *entries arrays* but the slots return
  `TF_Tensor_Handle*`; the tensor's delete path is unspecified.
- ⚠️ The "destroy is null-safe" rule is documented in `api.h` but not on the vtable `destroy`
  slots themselves.

### P4 — Every vtable carries ABI versioning
`size_t struct_size` as the first member **and** a `TF_*_STRUCT_SIZE` macro
(`TF_OFFSET_OF_END(...)`) so plugins can version-check.
- ✅ intern tier: `TF_BUFFER_STRUCT_SIZE`, `TF_TENSOR_STRUCT_SIZE`, `TF_SHAPE_STRUCT_SIZE`,
  `TF_DATATYPE_STRUCT_SIZE`, `TF_SHAPE_DATA_STRUCT_SIZE`, `TF_FILE_STATISTICS_STRUCT_SIZE`.
- ✅ fixed: `TF_*_STRUCT_SIZE` macros added to all 19 extern vtables (cache, cron, database, events, filesystem, generator, io, logger, worker_manager, orchestrator, otel, payload, profiler, protocol + protocol_server, search, serde, worker, env, registration) (filesystem, generator, cache, cron,
  database, events, logger, payload, serde, worker, manager, orchestrator, protocol, profiler,
  io, otel, env, env/thread, registration).

### P5 — One slot-naming dialect
Lowercase `snake_case` with `__` separating sub-object slots (`function__destroy`,
`span__set_attribute`).
- ✅ extern tier: `destroy`, `get_name`, `function__add_parameter`.
- ✅ fixed: intern slots renamed to the extern dialect (`allocate_tensor`, `get_name`, `tensor_bitcast_from`, `new_shape`, `data_type_size`…); `get_buffer` slot return type corrected to `TF_Buffer_Data`; the two bitcast slots gained `TF_Status*`.

### P6 — Context parameter: first, and uniformly named
Every vtable slot's first parameter is the owning context. Root backends: `plugin_context`.
Sub-objects: `<object>_context`.
- ✅ dominant: `plugin_context` (121×).
- ✅ fixed: intern `ctx` → `plugin_context`; io bare `request`/`response` → `*_context`; otel bare
  `tracer`/`meter`/`span`/`counter`/`histogram` → `*_context`; thread.h `arg` → `plugin_context`.
  (`file_context`/`region_context`/`server_context` already conformant.)
- ✅ fixed: `free_options` now takes `void* plugin_context` first.

### P7 — String convention
Inputs: `const TF_TString*`. Outputs: `TF_String* out`.
- ✅ fixed: generator.h (`set_name`, `enter_border_patrol`, `function__add_parameter`),
  filesystem.h (`writable_file__append`), env/dynamic_library.h now use `const TF_TString*`.
  ⚠️ registration.h is a **documented exception**: it is a char-keyed named registry (host-side,
  zero-copy keys), so its slots use `const char*` (was `const TF_String*`, the wrong spelling
  either way).

### P7-Naming — C ABI function names are `snake_case`, no `TF_` prefix
Every project-owned C ABI function (vtable slots AND free functions) is lowercase snake_case
without the `TF_` prefix: `get_buffer`, `allocate_tensor`, `init_generator`, `string_init`,
`new_status`, `start_thread`. The `TF_` prefix survives only on: legacy `api.h`/`api_experimental.h`
(upstream TensorFlow API), the type names (`TF_Status`, `TF_TString`, `TF_Buffer`), and enum
constants (`TF_OK`, `TF_FLOAT`, `TF_IO_GET`). ✅ applied (free functions renamed; slots already
snake; vtable members and free functions may share a name — struct member vs global, legal C).

### P8 — Every exported declaration is `TF_CAPI_EXPORT extern`
✅ fixed: `extern` dropped from ALL `TF_CAPI_EXPORT` declarations (0 remaining) — the redundant
  keyword is gone everywhere.

### P9 — One boolean spelling
✅ fixed: `TF_Bool` → `bool` in cron/database/manager/orchestrator/protocol (+`<stdbool.h>`);
  `unsigned char` remains only in api.h (legacy TF C API rule, documented).

### P10 — One enum-constant case
✅ fixed: `TF_FILESYSTEM_OPTION_TYPE_INT/_REAL/_BUFFER` (UPPER_SNAKE everywhere); enum type
  names stay PascalCase.

---

## 2. C++ builder tier — what a plugin author implements (`cc/abi/builder/**`)

### P11 — `create()` is "recover the instance from the opaque ctx"
Every vtable-backed class has `static X* create(void*) noexcept` (plus a const overload when
slots receive `const void*`), and its **own vtable lambdas use it** — the static_cast appears
exactly once, inside `create()`.
- ✅ 27 files conform; 166 lambda call sites use `X::create(ctx)`.
✅ fixed: filesystem vtable lambdas use `Filesystem::create(...)`; the three leaves
  (RandomAccessFile/WritableFile/ReadOnlyMemoryRegion) gained `create()` accessors and the leaf
  lambdas use them; every vtable-backed builder class now has the accessor.
- ✅ Correctly absent (not ctx-recovered): value types (AllocatorAttributes, FileStatistics,
  SearchQuery), host-only wrappers (ThreadView, DynamicLibrary), primitives (String, Status,
  TensorHandle, StringHive), PythonApiBuilder.

### P12 — Fallible methods return `std::expected<T, ice::Status>`
Infallible methods return plain values; never raw handles with NULL sentinels for fallible ops.
- ✅ extern-tier bases (Filesystem, Io, Otel, Cache, Events, …, Generator/Definition/Function…):
  `std::expected<T, ice::Status>` everywhere.
✅ fixed: intern builder bases return `std::expected<T, ice::Status>` for allocate_tensor,
  bitcast_from/to, new_buffer_from_string, new_buffer, new_shape; the bitcast C slots gained a
  trailing `TF_Status*`; allocator slots keep the raw-NULL P1 contract at the C boundary (lambda
  bridges down-level errors); sonic intern Runtimes match the new signatures; the 3 builder
  call sites (make_string_tensor, make_options_tensor, make_handle_tensor) handle the expected.

### P13 — `get_generic_vtable()`: `static`, returns the concrete `TF_X*`
✅ uniform (post-refactor).

### P14 — Identity: every backend base has `get_name()`
✅ uniform (post-refactor).

### P15 — Lowering: `to_c()` + `get_handle()`
`ice::String::to_c(TF_String*)`, `ice::Status::to_c(TF_Status*)`, `get_handle()` accessors —
the only way C++ values reach the C ABI.
✅ uniform.

---

## 3. C++ sonic tier — how the mainframe calls (`cc/abi/sonic/**`)

### P16 — `Runtime<T, Ops>` shape
`domain_name` static; constructor first (expanded); `get_name()`; no `this->`; Allman; owning
adapters with `m_ops && m_handle` dtor guard + deleted copy/move.
✅ uniform (post-refactor).

### P17 — Return locals directly (no `std::move` of a local)
✅ fixed: all 9 `return std::move(tf_x)` sites (incl. profiler) normalized to `return tf_x;`.

---

### P18 — Dependency direction: sonic NEVER imports builder
The sonic tier (mainframe-facing handles + C-ABI adapters) must never import or name any
`ice::builder::*` type — only the builder tier may depend on sonic. Shared enum/value types
(`DataTypeEnum`, `Method`, `PayloadType`, `SearchQuery`, …) live in `cc_abi_primitives` as
`ice::*`; sonic adapters are standalone `ice::sonic` classes that declare their interface
methods directly (no inheritance from builder bases, no `override`), speaking the flat C vtable
shape (e.g. `add_node(const void* def_context, …)`). ✅ applied (2026-08-29): all
`cc_abi_builder` imports/deps removed from sonic, adapters standalone, shared types moved to
primitives/types.cppm.

### P19 — Sonic names mirror builder names 1:1
A sonic adapter class mirrors its builder base's name (`ice::builder::Request` ↔ `ice::sonic::Request`,
`ice::builder::WorkerManager` ↔ `ice::sonic::WorkerManager`). The `Runtime` suffix is removed entirely —
sonic-only classes use plain domain names (`Time`, `Registration`). Every sonic class lives in
plain `ice::sonic` (no per-domain sub-namespaces). ✅ applied (2026-08-29 round 3).
✅ updated (2026-08-29): `Runtime` suffix removed entirely — `TimeRuntime` → `Time`,
`RegistrationRuntime` → `Registration`; duplicate `ThreadView`/`DynamicLibrary` classes
consolidated to one home each (`ice::builder::ThreadView`, `ice::sonic::DynamicLibrary`).

### P20 — Fallible methods are [[nodiscard]] and mirror const-ness
Every public method returning `std::expected<T, ice::Status>` is marked `[[nodiscard]]`; a sonic
method is `const` iff its mirrored builder base method is `const`. ✅ applied (2026-08-29 round 3).

### P21 — Shared value/enum types live in primitives
`ice::DataTypeEnum`, `ice::Method`, `ice::PayloadType`, `ice::SearchQuery` (+ `*_to_c`/`*_from_c`
converters) live in `cc_abi_primitives:types` — never duplicated per tier. Converters are always
named `<type>_to_c` / `<type>_from_c`. ✅ applied (2026-08-29 round 3).

## Open decisions (ABI-relevant or preference — need your call)
| # | Decision | Affected |
|---|----------|----------|
| D1 | Intern vtable slot naming: rename to lowercase `__` dialect, or keep `TF_`-prefixed as a documented second dialect? | tf_tensor/buffer/shape/datatype.h + builder/sonic intern |
| D2 | Intern builder bases: adopt `std::expected<T, Status>` (change `allocate_tensor` etc.), or declare them the raw/infallible exception? | builder/intern/*, sonic/intern/* |
| D3 | `TF_*_STRUCT_SIZE` macros: add to all 19 extern vtables, or drop from the intern tier? | c/extern/*.h |
| D4 | `extern` keyword: add to the 12, or drop everywhere? | api.h, api_experimental.h, tf_status.h |
| D5 | Boolean spelling: `bool` for all extern-tier code? | cron/database/manager/orchestrator/protocol headers |
| D6 | Enum constants: UPPER_SNAKE everywhere? | io/enums.h, option_types.h, otel/enums.h, … |
| D7 | By-value returns: `TF_GetOpList` → `TF_Buffer*`, `TF_GetBuffer` → pointer? | api.h, tf_buffer.h |
| D8 | `free_options` context param: add `void* plugin_context` (ABI change) or declare it the one stateless exception? | filesystem.h |

## Fixes that need no decision (apply on your go-ahead)
- F1 Filesystem vtable lambdas → `Filesystem::create(...)`; add `create()` to the three
  filesystem leaves.
- F2 `free_options`/getter ownership docs on the filesystem tensor-carrier slots.
- F3 Input-string spelling → `const TF_TString*` (generator, filesystem append, registration,
  dynamic_library).
- F4 `return std::move(tf_x)` → `return tf_x;` (8 sites).
- F5 Context-param naming → `plugin_context` / `<object>_context` (mechanical rename; ABI-safe).
