# C++ Rule of Five and Getter Convention Audit (`include/cc`)

This document records the repository-wide audit of all C++ header/module files under the `cc/` folder (`include/cc`), evaluating compliance with three critical conventions established in [CODESTYLE.md](file:///home/default/cc/congelado/CODESTYLE.md) and recent refactoring passes:

1. **Rule of Five**: All 5 special member functions must be explicitly declared (`= default;`, `= delete;`, or defined):
   - Destructor (`~T()`)
   - Copy Constructor (`T(const T&)`)
   - Copy Assignment Operator (`T& operator=(const T&)`)
   - Move Constructor (`T(T&&)`)
   - Move Assignment Operator (`T& operator=(T&&)`)
2. **Getter `noexcept` Guarantee**: Every getter method (`get_*` or `get()`) must be explicitly qualified with `noexcept`.
3. **Dual Getter Overload Pair**: For every property accessed via a getter, there must exist exactly two overloads:
   - One non-const: `T& get_name() noexcept`
   - One const: `const T& get_name() const noexcept`

---

## Executive Summary

| Category | `include/cc/utils` (24 files) | `include/cc/abi_gen` (20 files) | `include/cc/abi` (93 files) | `include/cc/tmp` (68 files) |
|---|---|---|---|---|
| **Incomplete Rule of Five** | 7 files | 4 files (+ 1 typo) | 65 files | 65 files |
| **Getters Missing `noexcept`** | 14 files | 8 files | 30 files | 7 files |
| **Getters Lacking Const & Non-Const Pairs** | 21 files | 7 files | 71 files | 7 files |
| **Failing All 3 Criteria Simultaneously** | 2 files | 0 files | 22 files | 7 files |
| **Fully Compliant Files** | 1 file ([`parser.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser.cppm)) | 7 files | 13 files | 2 files |

---

## 1. Active Modules: Compliance Matrix (`cc/utils` & `cc/abi_gen`)

| File | Class | Rule of 5 | Getters `noexcept` | Const & Non-Const Pairs |
|---|---|:---:|:---:|:---:|
| **`cc/utils/cli/ast/`** | | | | |
| [ast/command.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/ast/command.cppm#L8) | `Command` | ❌ Incomplete | ✅ All noexcept | ✅ All paired |
| [ast/flag.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/ast/flag.cppm#L9) | `Flag` | ✅ Complete | ✅ All noexcept | ❌ `get_has_value` (const only) |
| [ast/invocation.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/ast/invocation.cppm#L9) | `Invocation` | ✅ Complete | ✅ All noexcept | ❌ `get_current_command` (non-const only) |
| **`cc/utils/cli/basic/`** | | | | |
| [basic/argument.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/basic/argument.cppm#L9) | `Arguments` | ✅ Complete | ❌ `get_c_args` | ❌ `get_c_args` (const only) |
| [basic/command.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/basic/command.cppm#L9) | `Command` | ❌ Incomplete | ✅ All noexcept | ✅ All paired |
| [basic/record.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/basic/record.cppm#L10) | `Record` | ✅ Complete | ❌ `get_summary` | ❌ `get_duration`, `get_summary` |
| [basic/result.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/basic/result.cppm#L7) | `Result` | ❌ Incomplete | ✅ All noexcept | ❌ `get_exit_code`, `get_duration`, etc. |
| **`cc/utils/cli/command/`** | | | | |
| [command/arguments.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/command/arguments.cppm#L7) | `Arguments` | ✅ Complete | ❌ `get_args`, `get_c_args` | ❌ Both const only |
| [command/command.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/command/command.cppm#L10) | `Command` | ✅ Complete | ❌ All 3 getters | ❌ All 3 const only |
| [command/command_schema.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/command/command_schema.cppm#L8) | `CommandSchema` | ✅ Complete | ❌ Both getters | ❌ Both const only |
| [command/parser.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/command/parser.cppm#L10) | `Parser` | ✅ Complete | ❌ All 4 getters | ❌ All 4 const only |
| **`cc/utils/cli/parser/`** | | | | |
| [cli/executable.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/executable.cppm#L7) | `Executable` | ✅ Complete | ❌ `get_name`, `get_path` | ❌ Both const only |
| [cli/parser.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser.cppm#L7) | `Parser` | ✅ Complete | ✅ All noexcept | ✅ All paired |
| [parser/action.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser/action.cppm#L8) | `Action` | ❌ Incomplete | ✅ All noexcept | ✅ All paired |
| [parser/flag.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser/flag.cppm#L9) | `Flag` | ❌ Incomplete | ✅ All noexcept | ❌ `get_default_value`, `get_type` |
| [parser/helper.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser/helper.cppm#L28) | `FlagConfig` | ✅ Complete | ✅ All noexcept | ❌ `get_min`, `get_max` |
| [parser/helper.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser/helper.cppm#L125) | `ParserOptions` | ✅ Complete | ✅ All noexcept | ❌ `get_allows_unrecognized`, `get_has_auto_help` |
| [parser/option.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser/option.cppm#L9) | `Option` | ✅ Complete | ✅ All noexcept | ❌ `get_flag(string_view)` |
| [parser/schema.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/parser/schema.cppm#L9) | `Schema` | ✅ Complete | ✅ All noexcept | ❌ `get_option`, `get_flag` |
| **`cc/utils/cli/ (top-level)`** | | | | |
| [cli/record.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/record.cppm#L10) | `Record` | ✅ Complete | ❌ All 4 getters | ❌ All 4 const only |
| [cli/result.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/result.cppm#L10) | `Result` | ✅ Complete | ❌ All 6 getters | ❌ All 6 const only |
| [cli/runner.cppm](file:///home/default/cc/congelado/include/cc/utils/cli/runner.cppm#L10) | `Runner` | ✅ Complete | ❌ All 3 getters | ❌ All 3 const only |
| **`cc/utils/kernel/ & cc/utils/pipe/`** | | | | |
| [kernel/fd.cppm](file:///home/default/cc/congelado/include/cc/utils/kernel/fd.cppm#L13) | `UniqueFd` | ✅ Complete | ❌ `get_fd` | ❌ `get_fd` (const only) |
| [pipe/fd.cppm](file:///home/default/cc/congelado/include/cc/utils/pipe/fd.cppm#L12) | `UniqueFd` | ✅ Complete | ❌ `get` | ❌ `get` (const only) |
| [pipe/pipe.cppm](file:///home/default/cc/congelado/include/cc/utils/pipe/pipe.cppm#L13) | `Pipe` | ❌ Incomplete | ❌ Both getters | ❌ Both const only |
| [pipe/process.cppm](file:///home/default/cc/congelado/include/cc/utils/pipe/process.cppm#L14) | `Process` | ❌ Incomplete | ❌ All 6 getters | ❌ All 6 const only |
| **`cc/abi_gen/`** | | | | |
| [abi_gen/cli_options.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/cli_options.cppm#L8) | `CliOptions` | ❌ Incomplete | ➖ (No getters) | ➖ (No getters) |
| [abi_gen/cli_runner.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/cli_runner.cppm#L18) | `CliRunner` | ❌ Incomplete | ➖ (No getters) | ➖ (No getters) |
| [emitter/builder.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/generator/emitter/builder.cppm#L9) | `Builder` | ✅ Complete | ✅ All noexcept | ✅ All paired |
| [emitter/sonic.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/generator/emitter/sonic.cppm#L9) | `Sonic` | ✅ Complete | ✅ All noexcept | ❌ Missing `const` on 2nd overload |
| [helper/known_type.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/generator/helper/known_type.cppm#L14) | `KnownType` | ✅ Complete | ❌ All 4 getters | ❌ All 4 const only |
| [helper/include_finder.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/helper/include_finder.cppm#L11) | `IncludeFinder` | ✅ Complete | ❌ `const no` typo | ✅ Both overloads exist |
| [helper/parameter.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/helper/parameter.cppm#L9) | `Parameter` | ✅ Complete | ✅ All noexcept | ✅ All paired |
| [helper/paths.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/helper/paths.cppm#L9) | `DomainPaths` | ✅ Complete | ✅ All noexcept | ✅ All paired |
| [parser/parser.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/parser.cppm#L20) | `Parser` | ❌ Incomplete | ✅ All noexcept | ✅ All paired |
| [parser/registry.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/registry.cppm#L12) | `Registry` | ✅ Complete | ❌ `get_models` | ✅ Both overloads exist |
| [slot/reader.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/slot/reader.cppm#L15) | `Reader` | ⚠️ Incomplete (`= de;`) | ➖ (No getters) | ➖ (No getters) |
| [slot/slot.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/slot/slot.cppm#L11) | `Slot` | ✅ Complete | ✅ All noexcept | ✅ All paired |
| [vtable/ast_visitor.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/ast_visitor.cppm#L11) | `AstVisitor` | ✅ Complete | ✅ All noexcept | ✅ All paired |
| [vtable/model.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/model.cppm#L11) | `Model` | ✅ Complete | ❌ All 6 getters | ❌ All 6 unpaired |
| [vtable/naming.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/naming.cppm#L11) | `Naming` | ❌ Incomplete | ➖ (No getters) | ➖ (No getters) |
| [writer/helper/diff.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/writer/helper/diff.cppm#L7) | `DiffResult` | ✅ Complete | ❌ All 3 getters | ❌ All 3 const only |
| [writer/runner/diff.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/writer/runner/diff.cppm#L9) | `Diff` | ✅ Complete | ❌ `get_runner` | ❌ `get_runner` (const only) |
| [writer/runner/formater.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/writer/runner/formater.cppm#L10) | `Formatter` | ✅ Complete | ❌ `get_runner` | ❌ `get_runner` (const only) |
| [writer/writer.cppm](file:///home/default/cc/congelado/include/cc/abi_gen/writer/writer.cppm#L10) | `Writer` | ✅ Complete | ❌ `[[no]]` typo / missing | ❌ Both const only |

---

## 2. Detailed Findings by Criterion

### Criterion 1: Rule of Five Incomplete

The following **11 files** in `cc/utils` and `cc/abi_gen` have not completed the implementation of the Rule of Five:

1. [`include/cc/utils/cli/ast/command.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/ast/command.cppm#L8) — [`Command`](file:///home/default/cc/congelado/include/cc/utils/cli/ast/command.cppm#L8):
   - Has `~Command() = default;`
   - **Missing**: `Command(const Command&)`, `Command& operator=(const Command&)`, `Command(Command&&)`, `Command& operator=(Command&&)`
2. [`include/cc/utils/cli/basic/command.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/command.cppm#L9) — [`Command`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/command.cppm#L9):
   - Has `~Command() = default;`
   - **Missing**: `Command(const Command&)`, `Command& operator=(const Command&)`, `Command(Command&&)`, `Command& operator=(Command&&)`
3. [`include/cc/utils/cli/basic/result.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/result.cppm#L7) — [`Result`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/result.cppm#L7):
   - Has `~Result() = default;`, `Result(const Result&) = delete;`, `Result& operator=(const Result&) = delete;`
   - **Missing**: `Result(Result&&)`, `Result& operator=(Result&&)`
4. [`include/cc/utils/cli/parser/action.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/action.cppm#L8) — [`Action`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/action.cppm#L8):
   - Has copy deleted and move defaulted.
   - **Missing**: `~Action() = default;`
5. [`include/cc/utils/cli/parser/flag.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/flag.cppm#L9) — [`Flag`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/flag.cppm#L9):
   - Has copy deleted and move defaulted.
   - **Missing**: `~Flag() = default;`
6. [`include/cc/utils/pipe/pipe.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/pipe.cppm#L13) — [`Pipe`](file:///home/default/cc/congelado/include/cc/utils/pipe/pipe.cppm#L13):
   - Has copy deleted and move defaulted.
   - **Missing**: `~Pipe() = default;`
7. [`include/cc/utils/pipe/process.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/process.cppm#L14) — [`Process`](file:///home/default/cc/congelado/include/cc/utils/pipe/process.cppm#L14):
   - Has copy deleted and move defaulted.
   - **Missing**: `~Process() = default;`
8. [`include/cc/abi_gen/cli_options.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/cli_options.cppm#L8) — [`CliOptions`](file:///home/default/cc/congelado/include/cc/abi_gen/cli_options.cppm#L8):
   - **Missing**: all 5 special member functions (POD struct style).
9. [`include/cc/abi_gen/cli_runner.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/cli_runner.cppm#L18) — [`CliRunner`](file:///home/default/cc/congelado/include/cc/abi_gen/cli_runner.cppm#L18):
   - **Missing**: all 5 special member functions.
10. [`include/cc/abi_gen/parser/parser.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/parser.cppm#L20) — [`Parser`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/parser.cppm#L20):
    - Has `~Parser() = default;`
    - **Missing**: copy ctor, copy assign, move ctor, move assign.
11. [`include/cc/abi_gen/parser/vtable/naming.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/naming.cppm#L11) — [`Naming`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/naming.cppm#L11):
    - Has `Naming() = default;`
    - **Missing**: all 5 special member functions.
12. *(Defective declaration)* [`include/cc/abi_gen/parser/slot/reader.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/slot/reader.cppm#L15) — [`Reader`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/slot/reader.cppm#L15):
    - Line 23 contains syntax error: `Reader(Reader&&) = de;` (truncated `= delete;` or `= default;`).

---

### Criterion 2: Getters Missing `noexcept`

The following **22 files** contain getters that lack `noexcept`:

1. [`include/cc/utils/cli/basic/argument.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/argument.cppm#L50): `get_c_args`
2. [`include/cc/utils/cli/basic/record.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/record.cppm#L67): `get_summary`
3. [`include/cc/utils/cli/command/arguments.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/arguments.cppm#L36): `get_args`, `get_c_args`
4. [`include/cc/utils/cli/command/command.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/command.cppm#L58): `get_executable`, `get_arguments`, `get_stdin_text`
5. [`include/cc/utils/cli/command/command_schema.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/command_schema.cppm#L54): `get_name`, `get_flags`
6. [`include/cc/utils/cli/command/parser.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/parser.cppm#L78): `get_program_name`, `get_command`, `get_value`, `get_commands`
7. [`include/cc/utils/cli/executable.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/executable.cppm#L40): `get_name`, `get_path`
8. [`include/cc/utils/cli/record.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/record.cppm#L48): `get_command`, `get_result`, `get_duration`, `get_summary`
9. [`include/cc/utils/cli/result.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/result.cppm#L98): `get_exit_code`, `get_exited_normally`, `get_term_signal`, `get_std_out`, `get_std_err`, `get_duration`
10. [`include/cc/utils/cli/runner.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/runner.cppm#L36): `get_history`, `get_success_rate`, `get_mean_duration`
11. [`include/cc/utils/kernel/fd.cppm`](file:///home/default/cc/congelado/include/cc/utils/kernel/fd.cppm#L83): `get_fd`
12. [`include/cc/utils/pipe/fd.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/fd.cppm#L45): `get`
13. [`include/cc/utils/pipe/pipe.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/pipe.cppm#L108): `get_fd_read_end`, `get_fd_write_end`
14. [`include/cc/utils/pipe/process.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/process.cppm#L150): `get_fd_parent_write_stdin`, `get_fd_parent_read_stdout`, `get_fd_parent_read_stderr`, `get_fd_child_read_stdin`, `get_fd_child_write_stdout`, `get_fd_child_write_stderr`
15. [`include/cc/abi_gen/generator/helper/known_type.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/generator/helper/known_type.cppm#L82): `get_pointee_name`, `get_cpp_parameter_type`, `get_wrap_format`, `get_unwrap_format`
16. [`include/cc/abi_gen/parser/helper/include_finder.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/helper/include_finder.cppm#L119): `get_runner` const overload has typo `const no` instead of `const noexcept`
17. [`include/cc/abi_gen/parser/registry.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/registry.cppm#L71): `get_models` (both const and non-const lack `noexcept`)
18. [`include/cc/abi_gen/parser/vtable/model.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/model.cppm#L99): `get_pointee_type`, `get_struct_name`, `get_struct_size_macro`, `get_domain_name`, `get_class_name`, `get_slots`
19. [`include/cc/abi_gen/writer/helper/diff.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/helper/diff.cppm#L60): `get_identical`, `get_unified_diff`, `get_duration`
20. [`include/cc/abi_gen/writer/runner/diff.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/runner/diff.cppm#L69): `get_runner`
21. [`include/cc/abi_gen/writer/runner/formater.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/runner/formater.cppm#L68): `get_runner`
22. [`include/cc/abi_gen/writer/writer.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/writer.cppm#L80): `get_formatter` has typo `[[no]]` instead of `noexcept`; `get_diff_reporter` lacks `noexcept`

---

### Criterion 3: Missing Const & Non-Const Getter Pairs

The following **28 files** contain getters that are unpaired (only a `const` getter exists with no mutable counterpart, or vice-versa):

1. [`include/cc/utils/cli/ast/flag.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/ast/flag.cppm#L98): `get_has_value` (bool primitive, only const exists)
2. [`include/cc/utils/cli/ast/invocation.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/ast/invocation.cppm#L83): `get_current_command` (only non-const exists)
3. [`include/cc/utils/cli/basic/argument.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/argument.cppm#L50): `get_c_args` (only const exists)
4. [`include/cc/utils/cli/basic/record.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/record.cppm#L62): `get_duration`, `get_summary` (only const exists)
5. [`include/cc/utils/cli/basic/result.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/basic/result.cppm#L90): `get_exit_code`, `get_exited_normally`, `get_term_signal`, `get_duration` (scalars, only const exist)
6. [`include/cc/utils/cli/command/arguments.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/arguments.cppm#L36): `get_args`, `get_c_args` (only const exist)
7. [`include/cc/utils/cli/command/command.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/command.cppm#L58): `get_executable`, `get_arguments`, `get_stdin_text` (only const exist)
8. [`include/cc/utils/cli/command/command_schema.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/command_schema.cppm#L54): `get_name`, `get_flags` (only const exist)
9. [`include/cc/utils/cli/command/parser.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/command/parser.cppm#L78): `get_program_name`, `get_command`, `get_value`, `get_commands` (only const exist)
10. [`include/cc/utils/cli/executable.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/executable.cppm#L40): `get_name`, `get_path` (only const exist)
11. [`include/cc/utils/cli/parser/flag.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/flag.cppm#L88): `get_default_value`, `get_type` (only const exist)
12. [`include/cc/utils/cli/parser/helper.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/helper.cppm#L84): `get_min`, `get_max` in `FlagConfig`; `get_allows_unrecognized`, `get_has_auto_help` in `ParserOptions` (only const exist)
13. [`include/cc/utils/cli/parser/option.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/option.cppm#L169): `get_flag(string_view)` (lookup query, only const exists)
14. [`include/cc/utils/cli/parser/schema.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/parser/schema.cppm#L88): `get_option(string_view)`, `get_flag(string_view)` (lookup queries, only const exist)
15. [`include/cc/utils/cli/record.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/record.cppm#L48): `get_command`, `get_result`, `get_duration`, `get_summary` (only const exist)
16. [`include/cc/utils/cli/result.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/result.cppm#L98): all 6 getters (only const exist)
17. [`include/cc/utils/cli/runner.cppm`](file:///home/default/cc/congelado/include/cc/utils/cli/runner.cppm#L36): `get_history`, `get_success_rate`, `get_mean_duration` (only const exist)
18. [`include/cc/utils/kernel/fd.cppm`](file:///home/default/cc/congelado/include/cc/utils/kernel/fd.cppm#L83): `get_fd` (only const exists)
19. [`include/cc/utils/pipe/fd.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/fd.cppm#L45): `get` (only const exists)
20. [`include/cc/utils/pipe/pipe.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/pipe.cppm#L108): `get_fd_read_end`, `get_fd_write_end` (only const exist)
21. [`include/cc/utils/pipe/process.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/process.cppm#L150): all 6 `get_fd_*` getters (only const exist)
22. [`include/cc/abi_gen/generator/emitter/sonic.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/generator/emitter/sonic.cppm#L87): `get_writer`, `get_namespace_name`, `get_registry` — both declarations are non-const member functions (`const T& get_x() noexcept` without trailing `const`), leaving NO const getter overload.
23. [`include/cc/abi_gen/generator/helper/known_type.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/generator/helper/known_type.cppm#L82): `get_pointee_name`, `get_cpp_parameter_type`, `get_wrap_format`, `get_unwrap_format` (only const exist)
24. [`include/cc/abi_gen/parser/vtable/model.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/parser/vtable/model.cppm#L99): `get_pointee_type` (only non-const exists); 5 member getters (only const exist)
25. [`include/cc/abi_gen/writer/helper/diff.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/helper/diff.cppm#L60): `get_identical`, `get_unified_diff`, `get_duration` (only const exist)
26. [`include/cc/abi_gen/writer/runner/diff.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/runner/diff.cppm#L69): `get_runner` (only const exists)
27. [`include/cc/abi_gen/writer/runner/formater.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/runner/formater.cppm#L68): `get_runner` (only const exists)
28. [`include/cc/abi_gen/writer/writer.cppm`](file:///home/default/cc/congelado/include/cc/abi_gen/writer/writer.cppm#L80): `get_formatter`, `get_diff_reporter` (only const exist)

---

## 3. Files Failing All Three Criteria

In `cc/utils` and `cc/abi_gen`, exactly **2 files** fail all three criteria simultaneously:

1. [`include/cc/utils/pipe/pipe.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/pipe.cppm#L13) — `Pipe`:
   - ❌ Rule of Five: missing destructor (`~Pipe()`)
   - ❌ Getters `noexcept`: `get_fd_read_end`, `get_fd_write_end` lack `noexcept`
   - ❌ Dual overloads: only const getters exist
2. [`include/cc/utils/pipe/process.cppm`](file:///home/default/cc/congelado/include/cc/utils/pipe/process.cppm#L14) — `Process`:
   - ❌ Rule of Five: missing destructor (`~Process()`)
   - ❌ Getters `noexcept`: all 6 `get_fd_*` getters lack `noexcept`
   - ❌ Dual overloads: only const getters exist

---

## 4. Extended Modules Summary (`cc/abi` & `cc/tmp`)

For complete visibility across historical/vendored subtrees:

- **`include/cc/abi`** (93 files with classes):
  - Incomplete Rule of Five: **65 files**
  - Getters missing `noexcept`: **30 files**
  - Unpaired getters: **71 files**
  - Failing all three criteria: **22 files** (including `abi/builder/filesystem/filesystem.cppm`, `abi/builder/generator/generator.cppm`, `abi/sonic/plugin/dynamic_library.cppm`, `abi/sonic/filesystem/filesystem.cppm`)
- **`include/cc/tmp`** (68 files with classes, vendored TensorFlow runtime port):
  - Incomplete Rule of Five: **65 files**
  - Getters missing `noexcept`: **7 files** (`dataset.cppm`, `function.cppm`, `resource_op_kernel.cppm`, `cancellation.cppm`, `collective.cppm`, `model.cppm`, `variant_tensor_data.cppm`)
  - Unpaired getters: **7 files** (same 7 files)
  - Failing all three criteria: **7 files**
