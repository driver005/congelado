# Code Style Audit — `include/c` and `include/cc/abi`

Status: ✅ COMPLETE — all 12 decisions applied, formatted, and verified. See "Execution summary" below.

## Execution summary (what changed)

**Phase A — C ABI contract headers** (`include/c/**`):
- `c/extern/generator/generator.h`: `build`/`function__add_node`/`function__exit_border_patrol`
  now `void` (bool removed); `parameter__get_type` returns `void*` (was `const void*`);
  ownership contracts documented (`get_definitions`, `parameter__get_type`).
- `c/extern/filesystem/filesystem.h`: fully rewritten — `paths_exist` unified to a single
  `TF_Status*` with paths as a contiguous `const TF_TString*` array; `get_name` slot added;
  44 trailing-whitespace lines + whitespace-only runs removed.
- `c/extern/io/io.h`, `c/extern/otel/otel.h`: `get_name` slots added.
- `c/intern/tf_{tensor,buffer,shape,datatype}.h`: `TF_GetName` slots added.
- Pre-existing typedef conflicts fixed: vtable structs renamed `TF_Tensor`→`TF_TensorOps`,
  `TF_DataType`→`TF_DataTypeOps` (they had stolen the legacy public type names — files
  could not compile). Circular include `tf_tstring.h`→`tf_tensor.h` removed.
- `c/extern/env/env.h`: dangling `c/extern/env/time.h` include removed.
- ALL of `include/c` formatted with clang-format (repo config).

**Phase B — builder tier** (`include/cc/abi/builder/**`, 21 files):
- `get_generic_vtable()`: now `static` + concrete `TF_X*` everywhere (filesystem was
  `static const void*`); designated initializers + multiline lambdas everywhere (filesystem
  converted from a positional initializer that was silently off-by-one after the `get_name`
  insertion); `get_name()` pure virtual added to Filesystem/Io/Otel/Tensor/Buffer/Shape/DataType.
- `paths_exist` unified to `std::expected<void, ice::Status>`; vtable lambda matches the new C slot.
- `CompletionFn` wrapper + all conversion helpers deleted from cache/database/payload/worker/
  search/orchestrator; virtuals now take the C callback types directly.
- Generator vtable: bool slots → void (with proper early-return after error); `parameter__get_type` → void*.
- `builder/ops/` deleted (dead: no BUILD, included nonexistent `c/extern/ops.h`); dangling
  partitions (`:shape_handle`, `:dimension_handle`, `:thread_options_builder`, `:time`) removed
  from builder.cppm/env.cppm/thread_view.cppm; stale `cc_abi_value` → `cc_abi_primitives` in abi.cppm.
- python.cppm dangling `c/extern/python.h` → `c/abi/api.h`.

**Phase C — sonic tier** (`include/cc/abi/sonic/**`, 30 files):
- `PassNameToFactory` template parameter removed (was a no-op with TODO comments).
- Constructors moved to the top (second `public:` removed), expanded multi-line form.
- `this->` removed everywhere; dtors Allman multi-line with `m_ops && m_handle` guard
  (leaves: one-line dtors/ctors split, two-members-per-line split).
- `get_name()` added to Filesystem/Io/Otel/Tensor/Buffer/Shape/DataType Runtimes; all existing
  get_name methods normalized.
- `detail::to_c` completion conversions deleted; C callback types used directly.
- `sonic/filesystem/filesystem.cppm` repaired: illegal module fragment reordered; `paths_exist`
  rewritten for the new contract (contiguous TF_TString array); stale `free_string_array` calls
  and char**-array getters replaced with the tensor-carrier contract (opaque `ice::TensorHandle`
  pass-through, matching `Generator::get_definitions`).
- Generator partition `:controller` → `:runtime` (name/file match); protocol `Server`
  copy/move deletions added + `~ServerProtocol` renamed; otel `Tracer`/`Meter` →
  `Tracer`/`Meter`.
- `sonic/env/time.cppm`: nonexistent `c/extern/env/time.h` + `TF_NowNanos*` (declared nowhere)
  replaced with a real `clock_gettime` implementation (same "real host impl" pattern as
  `DynamicLibrary`'s dlopen/dlsym).
- `sonic/python.cppm`: dangling `c/extern/python.h` → `c/abi/api.h`.

**Phase D — format**: clang-format 22.1.8 (repo `.clang-format`) run over all of `include/c` +
`include/cc/abi`; output verified idempotent.

**Verification**: bazel build blocked by sandbox (output/install base read-only) — used instead:
all C headers pass `clang -fsyntax-only` (C) and `clang++ -std=c++23 -fsyntax-only` (C++);
`primitives:tensor_handle` module compiles to PCM; convention greps all zero
(PassNameToFactory, `this->m_ops`, completion helpers, `const void*` vtables, dangling refs);
vtable slot names cross-checked against C headers; stable_hlo (yoshi) call sites unaffected.

**Known pre-existing issues (NOT caused by this refactor, left as-is)**:
- `api.h` uses `bool` without `<stdbool.h>` in C mode (C++-only usage today).
- `api_experimental.h` declares `TFE_*` functions with undeclared types (eager layer removed).
- `safe_ptr.h` is C++-only (includes `<memory>`); `tensor_interface.h` includes nonexistent
  `proto/types.pb.h` — both likely dead/legacy, need a decision.
- `yoshi/omah_lay/stable_hlo/stable_hlo.cc` passes `ice::String` where `RegistrationRuntime::register_value`
  takes `const char*` (type mismatch, pre-existing).

## Part 1 — Format layer (clang-format)


Formatting deviations (braces, whitespace, columns, alignment) are fully inventoried below —
see the four subagent reports (builder 42 files, sonic 44 files, c/extern+intern 41 files,
primitives/ops/generator 14 files) which were merged into sections A–K. These are mechanical
and are resolved by "clang-format = truth + hand-fix what it can't express".

## Part 2 — Non-format layer (architecture / API design) ⭐

The user's actual concern. These are *semantic* inconsistencies — same concept expressed with
different shapes across the builder tier (what a plugin author implements), the sonic tier (how
the mainframe calls), and the C vtable headers (the contract between them).

### 2.1 Vtable return types / error contracts — 3 competing conventions
In the flat `c/extern` vtables, "did this call succeed?" is expressed three different ways:
1. **`bool` + `TF_Status*`** — `TF_Generator`: `build` (`generator.h:23`), `function__add_node`
   (`:29`), `function__exit_border_patrol` (`:37`).
2. **`TF_Status*` only, handle returned** — `get_definitions` (`generator.h:22`),
   `definition__get_inputs/outputs/attrs` (`:48-50`), `new_random_access_file` etc.
3. **`TF_Status*` only, `void` return** — `create_dir`, `delete_file`, `set_name`, …
Also: `parameter__get_type` returns `const void*` (`:57`) while ownership transfers (the sonic
adapter `const_cast`s — `sonic/generator/parameter.cppm:59-63`); `get_definitions` returns a
plugin-allocated tensor whose deletion contract is undocumented.
Chosen convention (user): **TF_Status* is the ONLY error channel**; slots return values or void,
never bool-for-success. ⇒ ABI change: the 3 bool slots in `TF_Generator` become `void`.

### 2.2 Vtable slot naming — two dialects
- Intern tier (`tf_tensor.h`, `tf_buffer.h`, `tf_shape.h`, `tf_datatype.h`): `TF_`-prefixed
  CamelCase members (`TF_AllocateTensor`, `TF_NewShape`) + `TF_*_STRUCT_SIZE` macros.
- Extern tier (`generator.h`, `filesystem.h`, `cache.h`, …): lowercase snake_case (`destroy`,
  `set_name`) + `__` sub-object slots (`function__add_parameter`, `writable_file__append`) and
  **no** struct-size macros.
Same concept ("vtable of function pointers"), two naming schemes, two ABI-versioning schemes.

### 2.3 get_generic_vtable() — three semantic inconsistencies
- **Return type**: `TF_Generator*` / `TF_Cache*` / `TF_Events*` / `TF_IO*` / `TF_DataType*` /
  `TF_Buffer*` … vs **`static const void*`** (`builder/filesystem/filesystem.cppm:121`).
- **static vs member**: `static` in 4 intern files (tensor/datatype/buffer/shape) + filesystem;
  non-static member in the other 16 (none of them touch `this`, so all are effectively static).
- **Init style**: designated+multiline (generator/cache/events), designated+one-line (io, otel,
  datatype), positional (filesystem).

### 2.4 Builder tier — what the plugin author implements
- **Tensor-runtime injection differs per class**: `Builder`/`Definition` store a *pointer* with a
  default ctor fallback (`m_tensor_runtime = nullptr`); `Filesystem` stores a *reference*
  (no default ctor, injection mandatory); cache/events/io/logger/… have **no tensor runtime at
  all**. Same "backend" concept, three shapes.
- **Identity API asymmetry**: `get_name()` exists in 15 builder classes (generator, cache, cron,
  database, events, logger, payload, worker, manager, orchestrator, protocol, search, attribute,
  parameter, definition) but is **absent** in filesystem, io, otel + the whole intern tier.
- **Concrete code inside abstract bases**: `Filesystem::make_string_tensor/make_options_tensor`,
  `Definition::make_handle_tensor`, `Builder::enter_border_patrol` (default impl) — while
  cache/events/etc. are pure interfaces. What belongs in the base vs the impl is not uniform.
- **`create()` accessor**: identical `static X* create(void*)` + same 3-line comment duplicated
  ~20×. Pattern-consistent, but screams for a shared CRTP base.
- **OpsBuilder is a 4th pattern**: owned-handle wrapper (not an abstract base), `alloc()` factory
  (vs `create`), `register_op()` ownership transfer — and includes the nonexistent
  `c/extern/ops.h`, has no BUILD (dead code).

### 2.5 Sonic tier — how the mainframe calls
- **Two call paths coexist**: sonic *Runtime* classes (Generator/Filesystem/Cache/…) call the
  flat vtable directly; sonic *adapter* classes (Function/Parameter/Attribute/TypeInfo,
  RandomAccessFile/WritableFile/…) implement the builder interfaces through the vtable. The
  Generator runtime then hands back a `std::reference_wrapper<ice::builder::Function>`
  (`runtime.cppm:68-83`) — half direct, half adapter, in one class.
- **Constructor placement**: bottom behind a second `public:` (generator/cache/worker/logger/io/
  filesystem Runtimes) vs top (all adapters). Chosen: top.
- **`this->` discipline**: `this->m_ops->…` in generator/cache/filesystem/worker/logger Runtimes,
  bare `m_ops->…` in tensor/buffer/shape/datatype + all adapters. Chosen: no `this->`.
- **PassNameToFactory**: intern tier spells bare `true` (`Runtime<Tensor, TF_Tensor, true>`),
  extern tier spells `/*PassNameToFactory=*/true` — and the parameter is a **no-op** (the
  `if constexpr` body is empty with TODO comments, `sonic/intern/runtime.cppm:53-58`).
- **Ownership guards differ**: dtor `if (m_ops && m_handle)` (generator adapters) vs
  `if (m_handle && m_ops)` (filesystem leaves) — same contract, different spelling.
- **Adapter hygiene gaps**: `protocol.cppm` `Server` is missing the deleted copy/move
  block; `~ServerProtocol` inside `Server` (`protocol.cppm:23`); otel member names
  `Tracer`/`Meter` vs `Tracer`/`Meter`.

### 2.6 Duplicated logic (should be one shared helper)
- The `CompletionFn ↔ TF_*_CompletionFn` conversion (static_assert + bit_cast) is copy-pasted:
  **5× in the builder tier** (cache, database, payload, worker, search — with inconsistent brace
  styles) **+ ~7× in the sonic tier** as `detail::to_c` (cache, search, database, payload,
  worker, orchestrator, otel). 12+ copies of the same conversion.
- The `// Recover the X instance…` comment + accessor: ~20 copies.
- `Status`/`String` handle plumbing (`ice::Status status;` → `status.get_handle()` →
  `if (!status.ok()) return std::unexpected{status};`) is consistent, good.

### 2.7 Cross-cutting semantics
- **`paths_exist`**: builder declares `std::vector<std::expected<void, ice::Status>>` while the C
  slot takes a tensor of per-path `TF_Status*` + a `TF_Status*` — two error models in one API;
  the sonic adapter calls the slot with **wrong arg types/count** (`sonic/filesystem/…:225-226`)
  so it cannot compile. `path_exists` (single status) vs `paths_exist` (per-path) asymmetry.
- **Fixed-width types**: `std::int64_t`/`std::size_t` vs bare `int64_t`/`size_t` mixed within
  files (cron, tensor, shape, filesystem, protocol).
- **`return *res` vs `res.value()` vs `res->…`** for expected unwrapping in vtable lambdas.
- **`alloc` vs `create`** static factories (ops_builder).
- **Namespace identity bug**: `string_hive.cppm` opens `namespace ice`, closes `// ice::sonic`;
  `search_query.cppm` opens `ice::builder::search`, closes `// ice::builder`.
- **Module-graph breakage** (beyond style): dangling partitions `:shape_handle`,
  `:dimension_handle`, `:thread_options_builder`, `:time` imported/exported but defined nowhere
  (builder.cppm, shape_inference_context_view.cppm, env.cppm, thread_view.cppm); stale
  `export import cc_abi_value;` in abi.cppm (module doesn't exist); partition `:controller` in a
  file named `runtime.cppm`; `sonic/filesystem/filesystem.cppm` global-module-fragment placed
  after `export module` (ill-formed).
- **Vtable slot naming `__`**: `random_access_file__destroy`, `server__destroy`,
  `span__set_attribute` — `snake__snake` convention unique to the extern tier, never explained.

## Decisions & execution status

User decisions (all 6 + 6 follow-ups answered):
1. clang-format = truth, then hand-fix patterns it can't express.
2. Function layout per .clang-format (short one-line, return type on own line when wrapping, params one-per-line).
3. Vtable building: designated initializers + multiline lambda body (generator/cache/events pattern).
4. TF_Status* is the ONLY error channel; get_generic_vtable() always returns concrete TF_X*.
5. Sonic adapters: ctor at top, no this->, Allman braces, multi-line dtors with m_ops && m_handle guard.
6. Style + structural fixes in the same pass.
7. bool→void for the 3 TF_Generator slots (ABI change accepted).
8. paths_exist unified to a single TF_Status* (per-path status vector dropped; paths travel as TF_TString* array).
9. CompletionFn wrapper + all 12+ conversion helpers REMOVED COMPLETELY — C callback types (TF_*_CompletionFn) used directly.
10. Dead code deleted (builder/ops, dangling partitions, cc_abi_value), sonic/filesystem module fragment repaired.
11. get_name() on every backend (filesystem/io/otel + intern tier TF_GetName slots).
12. PassNameToFactory template parameter removed (was a no-op).

Additional fixes made during execution:
- tf_tensor.h / tf_datatype.h had pre-existing typedef conflicts (vtable struct stole the legacy public type name).
  Vtable structs renamed to TF_TensorOps / TF_DataTypeOps.
- tf_tstring.h had a circular include of tf_tensor.h (unneeded) — removed.
- env.h included nonexistent c/extern/env/time.h — removed.
- Build blocked in this sandbox (bazel output/install base read-only); header-level syntax verification
  (clang/clang++ -fsyntax-only) used instead. Pre-existing issues found (not caused by the refactor):
  api.h uses `bool` without <stdbool.h> in C mode; api_experimental.h declares TFE_* functions with
  undeclared types (eager layer was removed); safe_ptr.h is C++-only; tensor_interface.h includes
  nonexistent proto/types.pb.h; yoshi/stable_hlo.cc passes ice::String where const char* is expected.



## Tier architecture (for context)

1. **`c/abi` + `c/intern`** — TensorFlow-derived C ABI types (TF_Status, TF_TString, TF_Tensor…).
2. **`c/extern/*`** — flat C vtables (`TF_Generator`, `TF_Filesystem`, `TF_Env`, …) + `TF_InitXxx` factory.
3. **`cc/abi/builder/*`** — abstract base classes (`ice::builder::X`) a plugin **implements**,
   each with the `static X* create(void*)` accessor and a `get_generic_vtable()` factory.
4. **`cc/abi/sonic/*`** — mainframe-facing handles (`ice::sonic::X` Runtime classes resolved via
   `RegistrationRuntime`) **plus** C-ABI adapter classes that implement `ice::builder::X` by
   calling through the flat vtable with `m_ops`/`m_handle`.

## Confirmed discrepancies (first-hand evidence)

### A. Function declaration layout — three competing layouts
Same conceptual construct formatted 3 different ways, often in the same class:
1. Return type on own line, name+params on next: `virtual std::expected<ice::TensorHandle, ice::Status>\n    get_definitions(ice::TensorHandle out) const = 0;`
2. One line (short): `virtual ice::String get_name() const = 0;`
3. Name + open paren, params one-per-line, close paren own line:
   `virtual std::expected<void, ice::Status> add_node(\n        const Definition& def,\n ...\n    ) = 0;`
Evidence: `include/cc/abi/builder/generator/function.cppm` (all three in one file),
`builder/filesystem/filesystem.cppm` (mixed), `builder/cache/cache.cppm` (mixed).

### B. Parameter alignment / wrapping
- `AlignAfterOpenBracket: BlockIndent` (one param per line) vs hand-aligned columns
  (`c/intern/tf_tensor.h` `TF_AllocateTensor` aligns `dims/num_dims/len` in a column) vs
  packed 2-per-line (`builder/filesystem/filesystem.cppm` `allocate_tensor(... String, &count, 1,`).
- Lambda continuation indent varies: 3-space (`builder/cache/cache.cppm`), aligned-under-first-param
  (`builder/generator/generator.cppm`), 4-space (`builder/filesystem/filesystem.cppm`).

### C. Vtable building — three competing styles
1. **Designated initializers, multiline lambdas**: `builder/generator/generator.cppm`,
   `builder/cache/cache.cppm`, `builder/events/events.cppm` (`.destroy =\n [](void* ctx) {`).
2. **Positional initializers**: `builder/filesystem/filesystem.cppm` (`{ sizeof(TF_Filesystem), [](...)`).
3. **Designated, one-line lambdas**: `builder/intern/datatype.cppm` (`.TF_DataTypeSize = [](...) -> size_t {`).
Also: `get_generic_vtable()` return type differs per file — `TF_Generator*` (generator),
`TF_Cache*` (cache), `TF_Events*` (events), `TF_DataType*` (datatype), `static const void*` (filesystem).

### D. Vtable return-type / error conventions (TF_* structs in c/extern)
- Mixed: `bool` + `TF_Status*` for success (`build`, `function__add_node`, `function__exit_border_patrol`)
  vs `TF_Status*` only (`definition__get_inputs`, `get_definitions`), vs `void*` + status (`enter_border_patrol`).
- Slot naming: `TF_Tensor` vtable uses `TF_AllocateTensor`-style names; `TF_Generator`/`TF_Filesystem`/
  `TF_Env` use lowercase `destroy`/`set_name`/`function__destroy`.
- Header brace style: **all 20 `c/extern` vtable headers** use `typedef struct TF_X {` (K&R),
  while `c/abi` + `c/intern` use Allman (`typedef struct TF_X\n{`).

### E. Builder base-class pattern inconsistencies
- `create()` accessor + identical comment block repeated in every class (consistent, but duplicated).
- Tensor-runtime injection: `Builder`/`Definition` store `m_tensor_runtime` as pointer (`= nullptr`),
  `Filesystem` stores a reference. Different null-safety stories.
- Constructor init-list layouts differ: colon-at-end-of-line + 8-space indent (generator.cppm,
  sonic adapters), colon-on-own-line + 4-space indent (`definition.cppm`), single-line delegating
  (`sonic/generator/runtime.cppm`, `sonic/cache/cache.cppm`).
- Member init syntax: braces `m_ops{ops}` vs parens `m_handle(handle)` (`primitives/tensor_handle.cppm`).

### F. Sonic adapter patterns
- Constructor placed at **bottom** behind a second `public:` (sonic/generator/runtime.cppm,
  sonic/cache/cache.cppm) vs at top (sonic/generator/function.cppm, attribute, typeinfo).
- `this->` explicit everywhere (generator runtime, cache, filesystem) vs bare members (tensor.cppm,
  leaves/random_access_file.cppm).
- Dtor guard order `m_ops && m_handle` (generator adapters) vs `m_handle && m_ops` (filesystem leaves).
- Filesystem leaves: single-line `~X() { if (...) ...; }` + **two members on one line**
  `private: TF_Filesystem* m_ops; void* m_handle;` — unique to that file.
- `Runtime<T, Ops, true>` — named comment `/*PassNameToFactory=*/true` (generator, filesystem, cache)
  vs bare `true` (tensor.cppm).

### G. Module fragment / import layout
- Correct order (module; → #includes → export module → imports) in most files, BUT
  `sonic/filesystem/filesystem.cppm` puts `module;` + includes **after** `export module` + `export import`
  (ill-formed) and has triple blank lines.
- `import std;` placement: mostly first import, but `sonic/filesystem/filesystem.cppm` places it after
  imports; some files omit it entirely (`tensor_handle.cppm`, `registration.cppm`).
- Include grouping violations: `env.h` has `#include "c/intern/tf_tstring.h"` after `<stdint.h>`.

### H. Brace style
- K&R vs Allman within the same file: `primitives/status.cppm` (`inline TF_Code status_code_to_c(...) {`
  vs Allman methods), `primitives/string.cppm` (`struct RuntimeState {`, `RuntimeState state() const {`),
  `sonic/intern/runtime.cppm` (`class Runtime {`, `virtual ~Runtime() {`),
  `sonic/registration/registration_runtime.cppm` (`struct RegistryState {`, `RegistryState() {`),
  `sonic/cache/cache.cppm` (`inline TF_Cache_CompletionFn to_c(...) {`).
- Single-line non-empty functions: `sonic/filesystem/leaves/*.cppm` destructors/ctors, `get_handle() const { return ...; }`
  (violates `AllowShortFunctionsOnASingleLine: Empty`).

### I. Whitespace / hygiene
- Trailing whitespace: `extern/filesystem/filesystem.h` (44 lines), `extern/otel/otel.h` (17), `extern/io/io.h` (13),
  plus many others; `status.cppm`, `string.cppm`, `sonic/intern/runtime.cppm` etc.
- Blank lines after opening braces: `primitives/string.cppm` (many ctors), `builder/ops/ops_builder.cppm`,
  **every method of `sonic/filesystem/filesystem.cppm`** (double blank line).
- Whitespace-only line runs: `extern/filesystem/filesystem.h` lines 41–111.
- Orphaned `);` / `)` on their own line: `sonic/filesystem/filesystem.cppm` lines 88–89, 99–100, 125–126…

### J. Namespace hygiene
- `primitives/string_hive.cppm` opens `export namespace ice {` but closes `} // namespace ice::sonic`.
- `safe_ptr.h` indents one namespace block and not the other; stale `namespace tensorflow`.
- `sonic/cache/cache.cppm`: non-export `namespace ice::sonic::detail` immediately followed by
  `export namespace ice::sonic {` with no blank line.

### K. Other structural issues (non-style, worth knowing)
- `builder/ops/ops_builder.cppm` + `shape_inference_context_view.cppm` include `"c/extern/ops.h"` which
  **does not exist**; the ops dir has no BUILD file (dead code).
- `sonic/filesystem/filesystem.cppm` `paths_exist` calls the vtable slot with wrong arg types/count
  (compiles against nothing) — the file is not in the current build.
- `builder/env/env.cppm` re-exports `:thread_options_builder`, `:thread_view`, `:time`, `:dynamic_library`
  but only `env/dynamic_library.cppm` and `env/thread_view.cppm` exist in `builder/env/` — missing files
  (`thread_options_builder.cppm`, `time.cppm`) or stale re-exports.
- BUILD file deps indentation broken in `sonic/filesystem/BUILD` (4 vs 8 spaces).

## Additional quantified evidence
- `TF_CAPI_EXPORT` without `extern`: 12 sites (`abi/api.h` ×2, `abi/api_experimental.h` ×9,
  `intern/tf_status.h` ×1).
- 20/20 `c/extern` vtable headers use K&R `typedef struct TF_X {`; 0 use Allman. The `c/abi`+`c/intern`
  tier uses Allman. Systematic split between tiers.
- Trailing whitespace: filesystem.h 44, otel.h 17, io.h 13, protocol.h 12, orchestrator.h 9, …
- Blank line after opening brace: sonic/filesystem 25, ops/shape_inference_context_view 11,
  sonic/python 10, sonic/protocol 9, sonic/io/leaves 7, primitives/string 7, …
- Single-line `if (!x) return …;` without braces: sonic/logger ×6.
- Column-aligned declarations: `intern/tf_buffer.h`, `intern/tf_shape.h` align `TF_*` names in a
  column; `intern/tf_tstring.h`/`tf_status.h` do not — the intern tier itself is split.
- Column-aligned params: `intern/tf_tensor.h` aligns `dims/num_dims/len`; `extern/generator.h`
  and others do not.
- `builder/env/env.cppm` re-exports `:thread_options_builder` and `:time`, which do not exist
  (BUILD has only `dynamic_library.cppm` + `thread_view.cppm`).
- `builder/ops/` has no BUILD, no root `ops.cppm`, and includes non-existent `c/extern/ops.h`.

## Open questions for the user (asked 2026-…)
TBD — see conversation.
