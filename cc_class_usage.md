# cc/ classes — who actually constructs/uses each one

Real grep evidence, not guesses. `Adder relevant?` = does this class even have a
vector member (only those are affected by public/private — everything else only ever
had setters regardless). Edit the **Your call** column (Public = keeps/gets a
chainable `add_x` adder · Private = bulk `set_x(vector<T>&&)` only) and send it back —
I'll apply whatever you land on before we continue.

## Legend for "Actually used from"
- **own file only** — nothing outside this one file ever touches it.
- **own module only** — used by other files, but all of them are in the same
  bazel module (e.g. two different `cc_abi_gen` files).
- **cross-module** — a *different* module (`cc_abi_gen` reaching into
  `cc_utils_cli`/`cc_utils_pipe`, or vice versa) constructs or calls it.

---

## cc/utils/cli — `ast::*` (the parsed-argv tree)

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `ast::Flag` | ast/flag.cppm | no (no vector) | `cli/parser.cppm`, `parser/schema.cppm` — **own module only**, never touched by `cc_abi_gen` | Private-shaped usage, but no adder exists anyway — moot | |
| `ast::Command` | ast/command.cppm | **yes** (3 vectors) | `cli/parser.cppm`, `parser/schema.cppm`, `parser/option.cppm` — **own module only**. `cc_abi_gen` only ever takes a `const ast::Command&` parameter, never constructs one | Private → bulk setters instead of 3 adders | |
| `ast::Invocation` | ast/invocation.cppm | **yes** (3 vectors) | `cli/parser.cppm`, `parser/schema.cppm` — **own module only**, never touched by `cc_abi_gen` | Private → bulk setters | |

## cc/utils/cli — process-invocation value types

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `Arguments` | basic/argument.cppm | **yes** (1 vector) | `basic/command.cppm` (own module) **and** `cc_abi_gen`'s `parser/parser.cppm`, `writer/runner/diff.cppm`, `writer/runner/formater.cppm` — **cross-module**, constructed directly by 3 abi_gen files | Public → keep `add_argument` | |
| `Command` (basic) | basic/command.cppm | no (no vector) | Constructed directly by `cc_abi_gen`'s `parser/parser.cppm`, `writer/runner/diff.cppm`, `writer/runner/formater.cppm` — **cross-module** | No adder either way — moot | |
| `Record` | basic/record.cppm | no (no vector) | Only constructed inside `Runner::execute()` — **own file only** | No adder either way — moot | |
| `Result` | basic/result.cppm | no (no vector) | Only constructed inside `Runner::execute()`/`manage_parent_io()` — **own file only**; `cc_abi_gen` only reads it via getters (`run_result->get_exit_code()` etc.), never constructs one | No adder either way — moot | |

## cc/utils/cli — engine/service types

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `Executable` | executable.cppm | no (no vector) | `basic/command.cppm`, `cli/runner.cppm` (own module) **and** `cc_abi_gen`'s `parser/parser.cppm`, `writer/runner/diff.cppm`, `writer/runner/formater.cppm` — **cross-module** | No adder — moot | |
| `Action<Args...>` | parser/action.cppm | no (no vector) | Only used as `Schema::m_action`'s type — **own file only**. `set_action()` is never called anywhere in the whole repo, dead API surface | No adder — moot | |
| `Parser` (cli) | parser.cppm | no (no vector) | Constructed by `cc_abi_gen/cli_runner.cppm` — **cross-module** | No adder — moot | |
| `Runner` | runner.cppm | **yes** (1 vector, `m_history`) — **already changed** | `Diff`, `Formatter`, `IncludeFinder` in `cc_abi_gen` all embed one and call `.execute()`/`.get_runner()` — **cross-module** | ⚠️ Already converted to `set_history()`, but this one **is** cross-module-used — worth double-checking against Public | |

## cc/utils/cli/parser — the schema-building DSL

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `parser::Flag` | parser/flag.cppm | no (no vector) | Built by `cc_abi_gen/cli_runner.cppm`'s `build_flag()` — **cross-module** | No adder — moot | |
| `FlagConfig` | parser/helper.cppm | **yes** (3 vectors) | Only referenced inside `parser/flag.cppm`'s `validate()` — **own file/module only**, never constructed by `cc_abi_gen` at all | Private-shaped usage, currently has adders (unchanged from first pass) | |
| `ParserOptions` | parser/helper.cppm | no (no vector) | **Not used anywhere** — defined, never referenced outside its own declaration | Dead class, no adder anyway — moot | |
| `Option` | parser/option.cppm | **yes** (2 vectors) | Built by `cc_abi_gen/cli_runner.cppm`'s `build_command_schema()` — **cross-module** | Public → keep `add_flag`/`add_subcommand` | |
| `Schema` | parser/schema.cppm | **yes** (2 vectors) | Built by `cc_abi_gen/cli_runner.cppm`'s `build_command_schema()` — **cross-module** | Public → keep `add_option`/`add_flag` | |

## cc/utils/pipe

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `UniqueFd` | pipe/fd.cppm | no (no vector) | Only `pipe/pipe.cppm` — **own module only** | No adder — moot | |
| `Pipe` | pipe/pipe.cppm | no (no vector) | Only `pipe/process.cppm` — **own module only** | No adder — moot | |
| `Process` | pipe/process.cppm | no (no vector) | `cli/runner.cppm` — **cross-module** (cli reaching into pipe) | No adder — moot | |

## cc/abi_gen — value/DTO-shaped

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `CliOptions` | cli_options.cppm | no (no vector) | `cli_runner.cppm` — **own module, different file** | No adder — moot | |
| `helper::KnownType` | generator/helper/known_type.cppm | no (no vector) | **Not used anywhere** — file isn't even wired into `generator/BUILD`, dead code | No adder — moot | |
| `parser::helper::Parameter` | parser/helper/parameter.cppm | no (no vector) | `parser/slot/reader.cppm`, `parser/slot/slot.cppm`, both emitters — **own module, several files** | No adder — moot | |
| `parser::helper::DomainPaths` | parser/helper/paths.cppm | no (no vector) | `cli_runner.cppm` — **own module, different file** | No adder — moot | |
| `parser::slot::Slot` | parser/slot/slot.cppm | **yes** (1 vector, `m_parameters`) | `parser/vtable/ast_visitor.cppm`, `parser/slot/reader.cppm`, `parser/vtable/model.cppm`, both emitters — **own module, several files, never touched outside cc_abi_gen** | Currently Public (has `add_parameter`) — arguable this is actually all-internal-to-abi_gen, no external module ever sees it | |
| `parser::vtable::Model` | parser/vtable/model.cppm | **yes** (1 vector, `m_slots`) | `cli_runner.cppm`, `ast_visitor.cppm`, both emitters, `registry.cppm` — **own module, several files, never touched outside cc_abi_gen** | Same question as `Slot` — currently Public (has `add_slot`), but nothing outside `cc_abi_gen` ever sees it either | |
| `writer::helper::DiffResult` | writer/helper/diff.cppm | no (no vector) | `writer/runner/diff.cppm`, `writer/writer.cppm` — **own module, different files** | No adder — moot | |

## cc/abi_gen — engine/service-shaped

| Class | File | Adder relevant? | Actually used from | My suggestion | Your call |
|---|---|---|---|---|---|
| `CliRunner` | cli_runner.cppm | no (no vector) | Constructed in `bin/main.cc` — **cross-module** (the binary's `main()`) | No adder — moot | |
| `generator::emitter::Builder` | generator/emitter/builder.cppm | no (no vector) | `cli_runner.cppm` — **own module, different file** | No adder — moot | |
| `generator::emitter::Sonic` | generator/emitter/sonic.cppm | no (no vector) | `cli_runner.cppm` — **own module, different file** | No adder — moot | |
| `parser::helper::IncludeFinder` | parser/helper/include_finder.cppm | no (no vector) | `parser/parser.cppm` — **own module, different file** | No adder — moot | |
| `parser::Parser` (abi_gen) | parser/parser.cppm | **yes** (1 vector, `m_system_arguments`) — **already changed** | `cli_runner.cppm` — **own module, different file, never touched outside cc_abi_gen** | ✅ Already converted to `set_system_arguments()` — matches "own module only" | |
| `parser::Registry` | parser/registry.cppm | map, not vector — **already changed** | `parser/parser.cppm`, `cli_runner.cppm`, both emitters — **own module, several files, never touched outside cc_abi_gen** | ✅ Already converted, adder removed | |
| `parser::slot::Reader` | parser/slot/reader.cppm | n/a (no members) | `parser/vtable/ast_visitor.cppm` — **own module, different file** | n/a | |
| `parser::vtable::AstVisitor` | parser/vtable/ast_visitor.cppm | no (no vector) | `parser/parser.cppm` — **own module, different file** | No adder — moot | |
| `parser::vtable::Naming` | parser/vtable/naming.cppm | no (no vector) | `parser/vtable/ast_visitor.cppm` — **own module, different file** | No adder — moot | |
| `writer::Diff` | writer/runner/diff.cppm | no (no vector) | `writer/writer.cppm` — **own module, different file** | No adder — moot | |
| `writer::Formatter` | writer/runner/formater.cppm | no (no vector) | `writer/writer.cppm` — **own module, different file** | No adder — moot | |
| `writer::Writer` | writer/writer.cppm | no (no vector) | `cli_runner.cppm` — **own module, different file** | No adder — moot | |

---

## The only rows where your call actually changes code

Everything marked "no adder — moot" or "n/a" stays exactly as-is no matter what you
pick, since there's nothing to switch between (only ever had setters). The rows that
actually matter:

1. **`ast::Command`, `ast::Invocation`** — currently have adders, but by the
   "cross-module usage" rule you gave me they'd be Private (never touched outside
   `cc_utils_cli`'s own parsing engine). Switch to bulk setters?
2. **`Arguments`** — cross-module confirmed, stays Public, no change.
3. **`FlagConfig`** — currently has adders, never constructed outside its own file.
   Switch to bulk setters?
4. **`Option`, `Schema`** — cross-module confirmed, stay Public, no change.
5. **`Slot`, `Model`** — currently have adders (`add_parameter`, `add_slot`), used
   across several `cc_abi_gen` files but never outside `cc_abi_gen` itself. This is
   the ambiguous case: multiple files, but all one module. Your call.
6. **`Runner`** — already converted to `set_history()`, but it turns out to be
   genuinely cross-module (`cc_abi_gen` embeds it in 3 places). Worth confirming
   that's still what you want given the "cross-module = Public" rule.
