# D Style Guide — congelado one_to_one port

Derived from `.clang-format` (LLVM-based, 4-space indent, 100 col), `.clangd`
naming conventions, and observed C++ code patterns.

---

## Formatting

| Rule | Value (from .clang-format) |
|---|---|
| Indent width | 4 spaces — never tabs |
| Column limit | 100 characters |
| Pointer/ref placement | East: `T* p` → D: same, `T* p` |
| Brace style | Opening brace on same line (no AfterClass/AfterFunction wrapping) |
| Max blank lines | 2 |

Configure dfmt:
```ini
[*.d]
indent_style = space
indent_size = 4
max_line_length = 100
```

---

## Naming Conventions

| Construct | Convention | Example |
|---|---|---|
| Types (class/struct/enum/interface) | PascalCase | `BufferNode`, `HpackEncodeView` |
| Functions / free functions | snake_case | `expand_written`, `register_class` |
| Variables (local) | snake_case | `written_bytes` |
| Member variables | snake_case with `m_` prefix | `m_tail`, `m_size`, `m_written` |
| Constants / enum members | UPPER_CASE | `MAX_FRAME_SIZE`, `END_STREAM` |
| Template parameters | PascalCase | `T`, `Self`, `Range` |
| Module names | lowercase dotted | `utils.buffering.node` |

**Do NOT switch to Phobos camelCase** even though Phobos uses it internally.

---

## Module Structure

Every D file begins with:
```d
module <dotted.path>;   // matches file path under src/
@nogc nothrow:          // applies to entire module
```

Then imports, then types, matching the order in the C++ source.

---

## C++ → D Construct Mapping

| C++23/26 | D |
|---|---|
| `export module X; import std;` | `module X;` + selective `import core.stdc.*;` / `@nogc`-safe Phobos |
| `namespace foo::bar { }` | `module foo.bar;` (flat module, no nesting needed) |
| `concept C = requires(T t) {...}` | template constraint `if (isInputRange!R && ...)` + `enum bool isC(T) = ...;` |
| `std::ranges` views, `range_adaptor_closure` | D input/forward ranges (`empty`/`front`/`popFront`); adaptors as classes (§0.6 rule); UFCS for piping |
| Deducing `this` (`auto op(this Self, ...)`) | `auto opCall(this This)(...)` or template `this` param |
| CRTP | template `this` parameter or `mixin` template |
| RAII / destructors | `class` with `~this()` dtor; `scope` for stack lifetime, `make!`/`dispose` for heap |
| `std::optional<T>` | `util.optional.Optional!T` (value struct, exempt from classes-only) |
| `std::expected<T,E>` | `util.result.Result!(T,E)` (value struct, exempt from classes-only) |
| `std::optional<std::reference_wrapper<T>>` | `T*` — PORT-NOTE: null = empty |
| Exceptions / `throw` | `Result!(T,E)` error codes; `@nogc nothrow` everywhere |
| `std::atomic<T>` + memory orders | `core.atomic` — preserve every release/acquire pairing exactly |
| `memory_order_acquire` | `MemoryOrder.acq` |
| `memory_order_release` | `MemoryOrder.rel` |
| `memory_order_acq_rel` | `MemoryOrder.acq` on load / `MemoryOrder.rel` on store |
| `memory_order_relaxed` | `MemoryOrder.raw` |
| `memory_order_seq_cst` | `MemoryOrder.seq` |
| `_pdep_u64` | `ldc.gccbuiltins_x86.__builtin_ia32_pdep_di` |
| `std::popcount` | `core.bitop.popcnt` |
| P2996 reflection / `describe_fields<T>()` | `__traits(allMembers, T)`, `__traits(getMember)`, `std.traits` at CTFE |
| `CONGELADO_PLUGIN()` macro | `mixin template CongeladoPlugin(...)` |
| Virtual interface classes (`ILogger`, `IServer`) | `extern(C++) interface` — no Object base, vtable-compatible |
| Placement new / manual lifetime | `core.lifetime.emplace`, `destroy!false`, malloc/free in `util.alloc` |
| `std::byte` | `ubyte` |
| `std::string` | `char[]` over caller-owned storage, or small string — no GC `string` |
| `std::unique_ptr<T>` | `make!T` + `dispose(t)` in owner's `~this()` |
| `std::shared_ptr<T>` with `acquire`/`release` | class with manual ref count via `core.atomic` |
| `std::span<T>` / `std::string_view` | `T[]` / `const(char)[]` — D slices are fat pointers |
| `static constexpr` table | `static immutable` array — CTFE-init, no GC allocation |

---

## Class-Only Rule (§0.6)

Every C++ class/struct with behavior → D `class`. Not `struct`.

**Exemptions (must have `// PORT-NOTE:`):**
1. ABI POD at C boundary → `extern(C) struct`
2. Value wrappers returned frequently on hot paths → `struct` (e.g. `Optional!T`, `Result!(T,E)`)

**Allocation paths (only these three):**
1. Stack: `scope auto obj = new C(args);`
2. Heap manual: `make!C(args)` / `dispose(obj)` from `util.alloc`
3. Pool/arena: `emplace` into pool-owned memory

Never `new C()` without `scope` — that's GC.

---

## Ranges

- No raw `for` where a range pipeline reads naturally
- Range adaptors are classes (§0.6 rule) with `empty`/`front`/`popFront`
- `save()` on a class range must deep-copy state. Add `// PORT-NOTE: class range, save() deep-copies`
- UFCS pipes for free: `range.myAdaptor(args)` works without closure boilerplate

---

## Import Ordering

1. `module` declaration
2. `@nogc nothrow:` module attribute
3. `import core.*` (D runtime core)
4. `import std.*` (@nogc-safe Phobos only)
5. `import ldc.*` (LDC intrinsics, platform-gated)
6. `import <project-module>.*` (other modules in this port)

---

## betterC Exceptions

Only these modules use `-betterC`:
- ABI surface: `sdk/plugin/plugin.d`, `sdk/worker/worker.d`

All others: full D with `@nogc nothrow`.
