# cc/ folder — requirements audit (v2, no exemptions)

Companion to `cc_requirements_checklist.md` (the row/column table) — this is the prose
version. Covers the same 46 files in `cc/utils` + `cc/abi_gen`. `cc/abi`, `cc/tmp`,
`cc/proto` remain out of scope per your own earlier calls — listed at the bottom, not
reviewed.

**Change from the previous version of this doc:** you said "nothing is exempt, all
need to have that pattern" — the 19 classes previously marked EXEMPT (RAII/factory/
engine types) now carry the pattern too. Two of them needed a real design change, not
just mechanical additions — flagged below and covered in more depth in the checklist's
"What changed" section.

## Legend

Same 11 requirements as before. ✅ = meets it · ➖ = N/A for this class (explained) ·
⚠️ = meets it with a documented, unavoidable deviation (a hard C++ constraint, not a
choice — e.g. `std::reference_wrapper` has no empty state).

---

## cc/utils/cli (13 files)

**ast/flag.cppm — `Flag`**: ✅ all applicable. No vector members.

**ast/command.cppm — `Command`**: ✅ all. 3 chainable adders. Fixed a dangling
`string_view` bug (`m_name` was a view bound to an rvalue temporary).

**ast/invocation.cppm — `Invocation`**: ✅ all. Same dangling-view bug fixed for
`m_program_name` and `m_global_operands`.

**basic/argument.cppm — `Arguments`**: ✅ all applicable (only a vector member, no
scalar).

**basic/command.cppm — `Command`** (process invocation): ⚠️ no default ctor — it
embeds `Executable`, which itself has no empty state to default into. Everything
else ✅.

**basic/record.cppm — `Record`**: ⚠️ same story, one level up (embeds `Command`).

**basic/result.cppm — `Result`**: ✅ all, no optional/vector so plain=all ctor.

**cli.cppm**: pure re-export index, no class.

**executable.cppm — `Executable`**: ✅/⚠️ mostly compliant with one deliberate
deviation: `set_name()` re-resolves `m_resolved_path` internally (a naive assignment
would desync the invariant), and there's intentionally **no mutable `get_name()&`** —
exposing one would let a caller mutate the name behind the path-resolution invariant's
back. Ctor/setter aren't `noexcept` (filesystem calls can throw).

**parser/action.cppm — `Action<Args...>`**: ✅ all. Single-member functor wrapper,
plain=all ctor already existed, added setter/getter.

**parser.cppm — `Parser`**: ✅ all. Added default ctor, `set_schema`, mutable
`get_schema()`.

**runner.cppm — `Runner`**: ✅ all applicable. Added `add_record` adder (chainable)
and a mutable `get_history()`. `execute()` itself stays non-`noexcept` deliberately —
it forks/execs and every failure path already surfaces through `std::expected`.

---

## cc/utils/cli/parser (2 files, 3 classes)

**parser/flag.cppm — `Flag`**: ✅ all (2=3 collapse). Enum-default-init bug fixed
(`m_type` had no initializer).

**parser/helper.cppm — `FlagConfig` + `ParserOptions`**: both ✅ all applicable. File
still holds 2 classes — pre-existing one-class-per-file violation, not fixed (separate
convention from your 11, flagged before, still not touched pending your say-so).

**parser/option.cppm — `Option`**: ✅ all. `execute()` made `const`.

**parser/schema.cppm — `Schema`**: ✅ all.

---

## cc/utils/pipe (4 files) + utils.cppm

**pipe/base.cppm**: re-export index, no class.

**pipe/fd.cppm — `UniqueFd`**: ✅ all applicable. Renamed `get()`→`get_fd()` for
convention (4 call sites in `pipe.cppm` fixed); added `set_fd()` which closes the
currently-held fd first — a naive setter would leak it.

**pipe/pipe.cppm — `Pipe`**: ✅ all. Added a public all-args ctor
(`Pipe(UniqueFd&&, UniqueFd&&)`) alongside the existing `create()` factory. Kept
`create()` as the recommended path — it's the only one that guarantees both fds came
from the same atomic `::pipe()` syscall; the public ctor exists for pattern
compliance and for callers who already have two unrelated fds to wrap.

**pipe/process.cppm — `Process`**: ✅ all, same factory-vs-ctor reasoning as `Pipe`.
Added `set_stdin/stdout/stderr` + `get_stdin/stdout/stderr` (mutable+const) alongside
the pre-existing derived int accessors (`get_parent_write_stdin` etc., left as-is —
cosmetic `noexcept`/`[[nodiscard]]` gap on those, not touched).

**utils.cppm**: empty stub module, no class.

---

## cc/abi_gen (23 source files)

**cc_abi_gen.cppm, generator/generator.cppm, parser/base.cppm, writer/base.cppm**:
pure re-export indexes, no classes.

**cli_options.cppm — `CliOptions`**: ✅ all (unchanged from the first pass — was never
exempt).

**cli_runner.cppm — `CliRunner`**: ✅ all now. Added a default ctor, an all-ctor
taking `writer::Writer&&`, and `set_writer`/`get_writer`. This only became possible
after making `Writer` movable (see below) — previously `Writer`'s move was deleted,
which would have made any ctor that moves one in ill-formed.

**bin/main.cc**: free `main()`, no class.

**generator/emitter/builder.cppm — `Builder`** / **sonic.cppm — `Sonic`**: ⚠️ no
default ctor is possible for either — both hold a `std::reference_wrapper<Registry>`,
and `std::reference_wrapper` has no default/empty state in the standard library; this
is a hard constraint, not a design choice. Everything else ✅: changed the
`namespace_name` param from `std::string_view` to `std::string&&` per the rvalue-move
rule (6 call sites in `cli_runner.cppm` updated to `std::string{NAMESPACE_NAME}`),
added setters/getters for both members plus the `m_writer` buffer.

**generator/helper/formater.cppm**: all free `inline` functions, no class —
pre-existing, not one of your 11, not touched.

**generator/helper/known_type.cppm — `KnownType`**: ✅ all (unchanged from first
pass). Still unwired in `generator/BUILD`'s partitions — dead code either way.

**parser/helper/include_finder.cppm — `IncludeFinder`**: ✅ all applicable. Only
member is a `Runner`; added `set_runner`/mutable `get_runner()`.

**parser/helper/parameter.cppm — `Parameter`**: ✅ all (unchanged, copy-not-move ctor
bug from the first pass stays fixed).

**parser/helper/paths.cppm — `DomainPaths`**: ✅ all applicable (unchanged, `const
T&&` un-movable-param bug stays fixed).

**parser/parser.cppm — `Parser`** (the clang-AST one): this was the hardest one.
It **threw `std::runtime_error` from its constructor**, and — this is the finding —
one of its three call sites (`run_generate`, `run_check` in `cli_runner.cppm`) called
it directly with no try/catch at all, meaning a failure there would have crashed the
binary with an unhandled exception. Converted to a non-throwing 2-string ctor plus a
`Parser::create(compiler_path, domain)` static factory returning
`std::expected<Parser, std::string>`, exactly matching the `Pipe::create()`/
`Process::create()` idiom already established in `cc/utils/pipe`. Rewrote all 3 call
sites in `cli_runner.cppm` to check the `expected` instead of catching an exception.
Also added the full adder/setter/getter set for its 6 members.

**parser/registry.cppm — `Registry`**: ✅ all now (was exempt as a "thin service").
`add()`→`add_model()`, `find()`→`get_model()` (5 call sites across `parser.cppm`,
`builder.cppm`, `sonic.cppm` fixed) — and along the way fixed the same
`const vtable::Model&&`-can't-move bug found in `DomainPaths`/`DiffResult` earlier.

**parser/slot/reader.cppm — `Reader`**: ✅ — has zero member fields, so there's
nothing to set/get/add; the default ctor is trivially the only meaningful one.

**parser/slot/slot.cppm — `Slot`**: ✅ all (unchanged from first pass).

**parser/vtable/ast_visitor.cppm — `AstVisitor`**: ✅ all applicable. Two plain
object members (`Naming`, `Reader`), added setters + mutable getters for both.

**parser/vtable/model.cppm — `Model`**: ✅ all (unchanged, const-correctness bug fix
stays).

**parser/vtable/naming.cppm — `Naming`**: ✅ all now. Added a plain ctor
(`explicit Naming(std::string&&)`) plus setter/getter for its one scratch-buffer
member.

**writer/helper/diff.cppm — `DiffResult`**: ✅ all (unchanged, `const T&&` bug fix
stays).

**writer/runner/diff.cppm — `Diff`** / **runner/formater.cppm — `Formatter`**: ✅ all
now. Both previously had copy *and* move deleted; changed to delete only copy,
default move — this was the enabling change for `Writer`'s all-ctor (below). Added a
regular ctor taking `Runner&&` plus `set_runner`/mutable `get_runner()`.

**writer/writer.cppm — `Writer`**: ✅ all now, same copy-deleted/move-enabled
treatment. Its all-args ctor moves in a `Formatter&&` and a `Diff&&` — this is what
required those two to become movable first; without that change the ctor would have
tried to invoke a deleted move constructor and failed to compile.

---

## Out of scope — unchanged from the previous version of this doc

`cc/abi` (128 files, excluded per your request), `cc/tmp` (106 files, declined —
vendored TensorFlow-style port), `cc/proto` (26 `.proto` schema files, not C++
classes), `cc/cc.cppm` (bazel target commented out, not even compiled). Full file
listings are in the previous version of this doc if you still want them; omitted here
to keep this update focused on what actually changed.

## Compile verification

`bazel build //include/cc/abi_gen/... //include/cc/utils/...` — green. Re-verified
after fresh-reading every one of the 18 files touched in the exemption-removal pass,
one at a time, before writing this update.
