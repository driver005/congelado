# cc_abi_gen

Autogenerates `include/cc/abi/{builder,sonic}/<domain>/<domain>.cppm` from the corresponding
`include/c/extern/<domain>/<domain>.h` vtable header, using real Clang AST (LibTooling), not
regex. Pilot scope: exactly two domains, `cache` and `logger` — both structurally simple (no
opaque handle, no injected members, no non-mechanical methods).

## Layout

One C++20 module per tier, each its own Bazel target (same pattern as
`include/cc/abi/{builder,sonic,primitives}`, composed by `import`, not by bundling everything
into one module's partitions):

- `parser/` (`cc_abi_gen_parser`) — Clang AST walk → `VtableModel`. The only place that
  `#include`s `<clang/...>` headers.
- `generator/` (`cc_abi_gen_generator`) — `VtableModel` → generated `.cppm` text
  (`BuilderEmitter`/`SonicEmitter`), plus `SlotClassifier`/`TypeRegistry`/`KnownType` (see
  below).
- `writer/` (`cc_abi_gen_writer`) — formats the generated text (`clang-format -style=file`) and
  either writes it to disk or diffs it against a real file.
- Top level (`cc_abi_gen_lib`/`cc_abi_gen`) — CLI parsing/orchestration (`cli_options.cppm`,
  `cli_runner.cppm`), importing the three tiers above.

Every class is instance-based (no `static` methods, no nested classes), one class per file. A
loop that accumulates a list either writes straight into the target `std::string` stream
(`BuilderEmitter`/`SonicEmitter`'s `m_writer`) or returns a non-owning `std::span` over storage
that already exists (`SlotClassifier::middle_parameters`) — never a throwaway local container.

## How it's wired

- **Automatic**: `include/cc/abi_gen/BUILD` has one `genrule` (`generate_pilot`) that runs
  `cc_abi_gen generate --pilot --out-dir <its own generated/ subdir>` — one process, covering
  every pilot domain in a single run. Each domain's own `BUILD` file (e.g.
  `include/cc/abi/builder/cache/BUILD`) points `primary_interface` directly at that genrule's
  output as a cross-package label (Bazel allows referencing another package's declared outputs;
  it only requires a rule's own outputs live within its own package — which is why this is one
  genrule in `cc_abi_gen`'s package, not one per domain package as an earlier version had it).
  Building any of those modules (or anything depending on them — i.e. the whole app) regenerates
  the whole pilot set. `GeneratedFileWriter` creates the `builder/<domain>/`/`sonic/<domain>/`
  subdirectories itself, since a genrule's output directory starts out empty every build.
- **Manual**: `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make
  gen-cc-abi`) regenerates the real, checked-in pilot files in place (uses Bazel's
  `BUILD_WORKSPACE_DIRECTORY` env var to find the real source tree — the standard idiom for a
  `bazel run` target that edits sources, as used by tools like buildifier/gazelle).
  `... check --pilot` (or `make check-cc-abi`) is the dry-run form: prints a unified diff against
  the checked-in files, writes nothing, nonzero exit on any mismatch.
  `--out-dir <dir>` (used by the genrule above) redirects `generate --pilot`'s output to an
  arbitrary directory instead of the real checked-in tree; `check --pilot` always diffs against
  the real tree regardless.

## Parsing rules

- The vtable struct is found structurally: a complete `RecordDecl` whose first field is named
  `struct_size` — not by matching `TF_<Name>` by name, so this generalizes without hardcoding.
- `TF_<NAME>_STRUCT_SIZE` is derived from the vtable struct's own name; never read from the
  preprocessor.
- Parameter names come from each field's `FunctionProtoTypeLoc` (the written declarator), since
  the canonical `FunctionProtoType` itself is name-erased.

## Slot classification (`generator/slot_classifier.cppm`)

1. `destroy: void(void*)` — absorbed into the destructor (builder) / `Runtime<T,Ops>`'s own
   destructor (sonic, no method emitted).
2. `get_name: void(void*, TF_String*)` — special-cased on both sides.
3. Any other slot whose last parameter's pointee is `TF_Status` — becomes
   `std::expected<void, ice::Status>` (the only shape `cache`/`logger` exercise; a
   non-void-return variant is documented but untested — needed by later rollout domains).
4. Every other pointer-shaped parameter type is looked up in the `TypeRegistry` (see below); a
   non-pointer type (a callback typedef like `TF_Cache_CompletionFn`, `void*`, an enum, a
   builtin) passes through unchanged.

## Type registry (`generator/type_registry.cppm`, `known_type.cppm`)

Every C intern/value type this generator knows how to wrap across the ABI boundary is one
`KnownType` entry — pointee name, the C++ parameter type it substitutes, and the wrap/unwrap
`std::format` patterns for the builder-lambda and sonic-call boundaries respectively (e.g.
`TF_TString` → `const ice::String&`, wrap `ice::String::create({})`, unwrap `{}.get_handle()`).
Seeded with `TF_TString`/`TF_String`/`TF_Status` — the ones the pilot actually exercises and that
have been verified against the real hand-written files.

A type a domain's header references but that isn't registered yet is queued in the registry's
pending list (not guessed at — it passes through unconverted for that run, which would be wrong
if it actually needed wrapping) instead of failing immediately. Once a domain is itself
generated, its own type is registered and cleared from pending. If anything is still pending
after every domain in the run has been processed, the run fails and lists what's missing — this
only works within one process (see `CliRunner::fail_if_types_pending`), which is exactly why the
build-time wiring is one consolidated genrule covering every domain rather than one genrule per
domain.

## Known intentional deviations from the pre-pilot hand-written files

Picking ONE canonical form (per `docs/style-audit.md`) rather than reproducing every existing
file byte-for-byte:
- Every vtable field, including the last, gets a trailing comma (the previous
  `builder/cache/cache.cppm` was missing it on `.remove`).
- Every `m_ops->method(...)` call is one line (the previous `sonic/cache/cache.cppm` had
  `.remove()` split across two lines).
- A fallible slot's builder-side lambda always binds an intermediate `self` variable (matches the
  existing `cache.cppm`'s style for `get`/`set`/`remove`; the previous `logger.cppm` chained
  `Class::create(plugin_context)->method(...)` directly, which clang-format ends up splitting
  across lines once the argument list is long enough — the very thing this convention avoids).

## Not yet handled (future rollout, ~20 remaining domains)

- Opaque `TF_<Domain>_Handle` types (detection exists in `parser/header_parser.cppm`,
  unexercised).
- Per-domain injected members (e.g. `filesystem`'s `ice::sonic::Tensor& m_tensor_runtime`) and
  non-mechanical methods (e.g. `generator`'s `create_function`) — these don't fit the
  `KnownType` wrap/unwrap model (a single-argument `std::format` pattern), since a generated
  vtable-domain type's own sonic wrapper takes a `(ops, plugin_context)` pair to construct, not
  one pointer.
- Registering the remaining `include/c/intern/*.h` value types (`TF_Tensor`, `TF_Shape`,
  `TF_Buffer`, `TF_DataType`, `TF_FileStatistics`, ...) in `TypeRegistry` — deferred rather than
  guessed at, since their exact wrap/unwrap convention needs verifying against the real
  hand-written `primitives/`/`sonic/intern/` files first.
- `scripts/check_vtables.py`'s regex-based drift checker should retire once `cc_abi_gen check`
  covers every domain it does.
