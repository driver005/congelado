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
  (`VTableEmitter`, `RuntimeEmitter`; see below).
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

## Slot rules

There are no special slot names, except the `create` slot on the builder side. Every slot becomes exactly one
`void` method; a `TF_Status*` parameter is an ordinary parameter (`const ice::sonic::Status&` in C++), filled by
the callee. There is no `std::expected`, no hidden temporary and no ownership in the generated code.

- Sonic (caller) methods are `const` and call `m_ops->slot(get_handle(), args...)`.
- Builder (implementer) callbacks resolve the object with `Class::from_handle(handle)` and call the virtual
  method with every pointer parameter wrapped by the builder's held ops
  (`self.wrap(std::type_identity<ice::sonic::X>{}, handle)`); the plugin fills the status itself.
- A pointer parameter whose pointee names a registered domain becomes `const ice::sonic::X&`; anything else
  passes through.

## Runtime (generated)

The only generated runtime piece is `ice::sonic::Runtime<Ops, Handle>` (partition `:runtime` of `intern/sonic`),
rendered by `RuntimeEmitter` from `runtime_base.inja`. There is no global state and no default backends, and
nothing in the runtime owns a plugin object.

- Runtime holds two members: `m_ops` (the plugin's ops table: what the object can do) and `m_handle` (the C
  handle struct by value: which object to do it on). Copying or moving it is trivial.
- Every ops struct in the C headers starts with `create(Handle*)` and `destroy(Handle*)`. The caller creates and
  destroys plugin objects with the ordinary slot methods `x.create()` and `x.destroy()`.
- The registry (`include/c/extern/registration/registration.h`) maps a type `String` and a provider `String` to an
  ops pointer; the caller builds those Strings (generated code never calls `copy`). `X{registry, type, provider}`
  looks the ops up with the registry's `get` slot and gives an empty handle; `X{registry, handle, type,
  provider}` and `X{ops, handle}` wrap a host handle (the struct is copied); `X{ops}` is the raw form used for
  the registry and String roots, which cannot look themselves up.
- A builder class holds the ops of the domains it wraps (`m_<Dependency>_ops`), is built with `X{deps_ops...}` and
  gets `get_generic_vtable(create)`: the plugin passes its `create` function (the one slot that cannot be dispatched
  through `from_handle`). `register_ops(registry, type, provider)` puts the ops table into the registry.
- The String, Status and registry backends are provided by plugins; without them nothing works.

A folder under `include/c/` becomes one C++ module, so keep the C headers acyclic at folder level: two folders
whose headers include each other (for example `memory/` and `stream_executor/` used to) cannot be built, and
belong in one folder. A domain's imports and BUILD `deps` are computed from the registry.

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
