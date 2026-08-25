# plugify's JIT call-generation engine and platform ops — how it works

This documents `src/jit/` and `src/platform/` from
[untrustedmodders/plugify](https://github.com/untrustedmodders/plugify) (MIT license), explained
in prose for reference rather than ported/reproduced. JIT/trampoline bugs are
memory-corruption-class, not compile-error-class — a wrong register or stack-slot assignment
doesn't fail to compile, it silently corrupts state at runtime — which is why this is documented
rather than directly transcribed.

## The problem this code solves

Plugify's plugins export functions whose signatures (return type + parameter types) are only
known at *runtime*, described by a `Method`/`Signature` descriptor loaded from a plugin's
manifest — not at compile time as a C++ function pointer type. To actually call such a function
(or receive a call *into* such a function from native code that doesn't know about plugify),
something has to bridge "I have a signature description and an address" to "here's a real,
callable native function with that exact calling convention." That bridge is generated at
runtime as actual machine code — a JIT-compiled trampoline — once per unique signature, then
reused for every call through it.

Two directions, two files per architecture:
- **`call.cpp`** (`JitCall`) — host calls *into* a plugin's exported function. Takes a flat array
  of argument values, generates a stub that loads them into the right registers/stack per the
  platform ABI, calls the real target address, and writes the return value back into a
  caller-owned slot.
- **`callback.cpp`** (`JitCallback`) — the reverse: generates a native-ABI-conforming stub that,
  when called by *anyone* (including code with no idea plugify exists), collects the incoming
  arguments out of registers/stack into a flat array and invokes a plugify-side callback handler
  with that array, then marshals the handler's result back into the return value the original
  caller expects.

## The key architectural fact: this isn't hand-rolled machine code

Both directions are built on **asmjit**, a real third-party JIT/code-generation library (a
dependency of this part of plugify, not something written from scratch here). Specifically, its
high-level `Compiler` API (`x86::Compiler` / `a64::Compiler`) — which itself already knows how to
allocate registers, honor calling conventions, and emit the actual instruction bytes for a given
`FuncSignature`. Plugify's own code is a layer *on top* of that: it converts its own `Signature`
type into an asmjit `FuncSignature` (`ConvertSignature` in `jit/helpers.hpp`), then uses asmjit's
`invoke()`/`add_func()` calls to describe the marshaling logic (move argument N from this flat
buffer slot into this virtual register; asmjit's own register allocator decides the real
hardware placement). Concretely, this means plugify isn't manually choosing which hardware
register holds which argument or manually encoding `mov`/`call` opcodes — it's describing *what*
needs to move where, in terms of asmjit's virtual-register/memory abstractions, and asmjit's own
compiler backend handles the actual encoding and register allocation.

## `jit/helpers.hpp`

Shared utilities both `x86/` and `a64/` implementations use:
- A small generic helper mapping a C++ type to asmjit's own `TypeId` enum (used for the flat
  parameter-buffer pointer/count/handle types the trampolines themselves take, as opposed to the
  *target* function's own argument types).
- Conversions from plugify's own `ValueType` enum (its closed set of marshalable FFI types — see
  `include/plugify/value_type.hpp`) to asmjit's `TypeId`, and from plugify's `CallConv` enum to
  asmjit's `CallConvId` — i.e. translating plugify's own type/ABI vocabulary into asmjit's.
- `ConvertSignature`: builds a complete asmjit `FuncSignature` from a plugify `Signature` by
  running the above per-argument conversion across the whole parameter list.
- A minimal `asmjit::ErrorHandler` subclass that just records the first error asmjit reports
  (asmjit itself validates things like "this type doesn't fit in a single register" and reports
  failures through this interface, rather than plugify's code validating that itself).

Neither this file, nor `x86/helpers.cpp`/`a64/helpers.cpp` below, does any code generation
themselves — they're pure lookup-table/plumbing code translating between two type systems
(plugify's `ValueType`, asmjit's `TypeId`). `x86/helpers.cpp` and `a64/helpers.cpp` each also
carry one small architecture-specific fact: whether a given argument type needs *two* register
slots on this architecture (relevant on 32-bit x86, where a 64-bit integer argument doesn't fit
in one register and needs a high/low pair — irrelevant on 64-bit architectures, where every
general-purpose register is already 64 bits wide).

## x86 JIT (`jit/x86/call.cpp`, `callback.cpp`, `helpers.cpp`)

Targets the System V AMD64 (Linux/macOS) and Microsoft x64 calling conventions (asmjit itself
knows the exact register/stack rules for each; plugify just tells it which convention via
`CallConvId`). The general shape of both `call.cpp` and `callback.cpp`:

1. Convert the plugify `Signature` to an asmjit `FuncSignature`.
2. Open a new asmjit compiler context and start describing a function body.
3. Walk the argument list one type at a time, classifying each as either an integer-class value
   (goes in a general-purpose virtual register) or a floating-point-class value (goes in an
   XMM/vector virtual register) — asmjit's own `TypeUtils` provides this classification; plugify
   just branches on it to decide which kind of virtual register to allocate and which `mov`
   variant to emit (integer move vs. `movq` for the vector register).
4. For `call.cpp`: read each argument out of a flat caller-supplied buffer into its virtual
   register, then use asmjit's `invoke()` to call the real target address with those registers
   mapped to the real argument slots (asmjit places them in the correct physical registers per
   the calling convention). For `callback.cpp`: the reverse — the generated function's own
   parameters (populated by whoever calls it, following the real ABI) get copied out into a
   stack buffer, and *that* buffer plus a method pointer and user-data pointer get passed to
   plugify's own callback handler function.
5. Handle the return value the same way in reverse: classify the return type, and if it doesn't
   fit in a single register, split it across a register pair (e.g. two 32-bit halves on 32-bit
   builds, or the high/low 64-bit halves of a 128-bit vector return on 64-bit builds) or fall
   back to the **hidden-return-pointer (sret) convention** — the caller pre-allocates the return
   storage and passes a pointer to it as an extra hidden argument, and the generated code just
   writes through that pointer instead of returning a value in a register at all. This is the
   same sret mechanism plugify's calling convention already uses for any "object" type (strings,
   vectors, etc. — see the earlier `IsHiddenParam`/`ValueType::_ObjectStart..._ObjectEnd` design
   noted elsewhere in this project's own history with plugify's ABI).
6. Anything wider than 64 bits per individual argument/return slot that doesn't fit asmjit's
   handling (noted in the source as a known asmjit limitation, not a plugify-specific one) is
   rejected with an explicit error rather than silently mishandled.
7. Finalize the compiler (asmjit emits the actual machine code into an executable buffer at this
   point) and hand back the resulting callable address; the `JitRuntime`/`CodeHolder` objects
   from asmjit own that executable memory's lifetime (released when the `JitCall`/`JitCallback`
   is destroyed).

`callback.cpp`'s extra complexity over `call.cpp`: since it's building a function that *receives*
a call from arbitrary native code (not one plugify itself invokes), it also has to build a
temporary on-stack buffer to stage the incoming arguments into a flat, uniform layout before
calling the plugify-side handler (which expects "pointer to an array of uint64-sized slots plus
a count," not "whatever mix of registers the real ABI happened to use") — and then, if the
handler produces a value, copy it back out of that buffer into whatever registers the *original*
caller is expecting the return value in.

## arm64 JIT (`jit/a64/call.cpp`, `callback.cpp`, `helpers.cpp`)

Same two-file structure and the same overall approach (classify arguments, allocate virtual
registers via asmjit's `a64::Compiler`, marshal to/from a flat buffer, handle sret for oversized
returns), targeting AAPCS64 instead of System V AMD64/Microsoft x64. The concrete differences
that follow from that:
- AAPCS64's integer registers are natively 64 bits wide, so the "does this argument need a
  second, high-order register slot" question that matters on 32-bit x86 doesn't arise at all —
  every integer-class argument up to 64 bits fits in one register, unconditionally.
  `a64/helpers.cpp`'s equivalent of that classification helper reflects this (effectively always
  "no" rather than x86's conditional check).
- Register naming/classes differ (AArch64 general-purpose vs. SIMD/FP registers vs. x86's
  general-purpose vs. XMM registers) — asmjit's `a64::Compiler` API surfaces AArch64's own
  register types, so the actual virtual-register allocation calls differ syntactically from the
  x86 versions, but conceptually play the identical integer-vs-float-class role.
- Load/store syntax differs (AArch64 uses explicit `ldr`/`str`-style instructions instead of a
  unified `mov`), which shows up as different asmjit compiler method calls for what is
  functionally the same "move this value between a register and memory" operation.

## `platform/unix_ops.cpp` and `platform/windows_ops.cpp`

Both implement the same `IPlatformOps` interface (declared in `include/plugify/platform_ops.hpp`,
not covered by this document) — plugify's abstraction over OS-level dynamic-library loading, so
the rest of the engine never calls `dlopen`/`LoadLibrary` directly. The interface covers:
loading a library from a path (with a set of `LoadFlag` options — lazy vs. immediate symbol
binding, global vs. local symbol visibility, whether to allow the library to be unloaded, etc.),
unloading a library, resolving a symbol by name, and recovering the filesystem path a loaded
library handle actually corresponds to.

- **`unix_ops.cpp`** implements this directly on top of POSIX `dlopen`/`dlclose`/`dlsym`/`dladdr`
  (via `<dlfcn.h>`), translating plugify's own `LoadFlag` bits into the corresponding `RTLD_*`
  flags (lazy → `RTLD_LAZY` vs. `RTLD_NOW`, global-symbols → `RTLD_GLOBAL` vs. `RTLD_LOCAL`, and
  a couple of flags that map to platform-conditional `RTLD_NODELETE`/`RTLD_DEEPBIND` extensions
  that aren't defined on every Unix). Symbol-path recovery uses `dladdr()`. It reports itself as
  not supporting runtime search-path modification but does support lazy binding.
- **`windows_ops.cpp`** implements the same interface on the Win32 API (`LoadLibrary`/
  `FreeLibrary`/`GetProcAddress`/`GetModuleFileName`-family calls), which is structurally
  different from the POSIX dynamic-loader API (no direct equivalent of `RTLD_LAZY`, different
  error-reporting convention via `GetLastError()` rather than `dlerror()`, no `dladdr`-style
  reverse lookup — Windows instead needs `GetModuleHandleEx`-style APIs to go from an address
  back to a module). The file also does a large amount of Win32-header hygiene at the top (a long
  block of `#define NO*`-style macros before including the real Windows headers) purely to
  suppress unrelated Win32 API surface area (GUI/menu/message-related macros) that would
  otherwise pollute the global namespace — not functionally part of the dynamic-loading logic
  itself.

## `src/pch.hpp`

A precompiled header: a flat list of common standard-library includes (`<algorithm>`, `<vector>`,
`<unordered_map>`, `<filesystem>`, `<mutex>`, `<ranges>`, etc.) plus `plugify/global.h`, used
purely to speed up compilation across the many translation units in `src/` — nothing to explain
architecturally.

---

**Coverage**: 10 files (`jit/helpers.hpp`; `jit/x86/{call,callback,helpers}.cpp`;
`jit/a64/{call,callback,helpers}.cpp`; `platform/{unix_ops,windows_ops}.cpp`; `pch.hpp`),
2,108 source lines total.
