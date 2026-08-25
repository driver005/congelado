Structured class/function index for plugify's src/jit/ and src/platform/ (MIT license, github.com/untrustedmodders/plugify) — signatures and one-line purposes only (no register/encoding-level detail), companion to src-jit-platform.md's prose explanations.

### src/jit/helpers.hpp

**Free functions / templates** (namespace `plugify`)
- `template<typename T> asmjit::TypeId GetTypeIdx()` — maps a C++ type to asmjit's runtime `TypeId` enum value.
- `bool HasHiArgSlot(asmjit::TypeId)` — reports whether a value needs a second ("high") register slot on this architecture.
- `asmjit::TypeId GetValueTypeId(ValueType)` — maps plugify's own `ValueType` enum to an asmjit `TypeId` for a parameter.
- `asmjit::TypeId GetRetTypeId(ValueType)` — same mapping, specialized for return-value positions (return rules differ from argument rules).
- `asmjit::CallConvId GetCallConvId(CallConv)` — maps plugify's `CallConv` enum to asmjit's calling-convention identifier.
- `asmjit::FuncSignature ConvertSignature(const Signature&)` — builds an asmjit function signature (return type + arg types + calling convention) from plugify's own `Signature` type.

**`SimpleErrorHandler`** (extends `asmjit::ErrorHandler`)
- `void handle_error(asmjit::Error, const char*, asmjit::BaseEmitter*) override` — captures the first asmjit code-generation error into member fields instead of asmjit's default behavior.

### src/jit/x86/call.cpp

**`JitCall`** (public class declared in `include/plugify/call.hpp`; this file is its x86 implementation, via a private `Impl` PIMPL struct)
- `JitCall()` — constructs, allocating the `Impl`.
- `JitCall(JitCall&&) noexcept` — move constructor (defaulted).
- `~JitCall()` — releases the generated trampoline from the shared `asmjit::JitRuntime` if one was built.
- `JitCall& operator=(JitCall&&) noexcept` — move assignment (defaulted).
- `Address GetJitFunc(const Signature&, Address target, WaitType, bool hidden)` — builds (or returns the cached) call trampoline for the given signature and target function pointer; `hidden` marks a hidden sret-style return parameter.
- `Address GetJitFunc(const Method&, Address target, WaitType, HiddenParam)` — convenience overload: derives a `Signature` from a `Method` descriptor (resolving hidden-return classification via the `HiddenParam` predicate) and calls the signature-based overload.
- `Address GetFunction() const noexcept` — returns the generated trampoline's entry address.
- `Address GetTargetFunc() const noexcept` — returns the target function address the trampoline calls into.
- `std::string_view GetError() noexcept` — returns the last code-generation error message, if generation failed.

`JitCall::Impl` (private, x86-specific)
- `Address GetJitFunc(...)` — the actual x86 trampoline-building routine; walks the signature's argument list, classifies each argument by the SysV/Windows x86 ABI's register-vs-stack rules, and emits code (via asmjit's `x86::Compiler`) that marshals a generic `(void* args, void* ret)` call into the target's real native calling convention, then invokes it.

### src/jit/x86/callback.cpp

**`JitCallback`** (public class declared in `include/plugify/callback.hpp`; x86 implementation via PIMPL)
- `JitCallback()` / `JitCallback(JitCallback&&) noexcept` / `~JitCallback()` / `operator=(JitCallback&&) noexcept` — same lifecycle shape as `JitCall`.
- `Address GetJitFunc(const Signature&, const Method*, CallbackHandler, Address data, bool hidden)` — builds (or returns cached) a *reverse* trampoline: native code matching the target signature that, when called by a plugin, marshals the incoming native arguments into a generic `(method, userData, argsPtr, argCount, retPtr)` call and invokes the host-side `CallbackHandler`.
- `Address GetJitFunc(const Method&, CallbackHandler, Address data, HiddenParam)` — convenience overload deriving `Signature` from a `Method`, same pattern as `JitCall`.
- `Address GetFunction() const noexcept` — the generated callback trampoline's entry address (this is what gets handed to a plugin as "the native function pointer").
- `Address GetUserData() const noexcept` — the opaque user-data pointer threaded through to `CallbackHandler`.
- `std::string_view GetError() noexcept` — last code-generation error, if any.

`JitCallback::Impl` (private, x86-specific) — mirrors `JitCall::Impl`'s approach in the opposite direction: builds a stack-based argument buffer from the incoming native arguments/registers, invokes the host `CallbackHandler` with pointer+count to that buffer, then marshals the handler's result back into the native return convention.

### src/jit/x86/helpers.cpp

Implements the four free functions declared in `helpers.hpp`, for the x86 architecture specifically:
- `HasHiArgSlot` — returns `true` only on 32-bit builds for 64-bit integer types (which need two 32-bit register halves); always `false` on 64-bit x86.
- `GetValueTypeId` — full switch over every `ValueType` enumerator (scalars map to their matching asmjit `TypeId`; strings/pointers/arrays/vectors/matrices all map to a generic pointer-width type).
- `GetRetTypeId` — a separate, architecture-and-bit-width-conditional switch (has distinct branches for 64-bit vs 32-bit builds, and Windows vs non-Windows return-register conventions for `Vector2/3/4`).
- `GetCallConvId` — a straight one-to-one mapping table from plugify's `CallConv` enumerators to asmjit's, with `CDecl` as the fallback for unrecognized values.

### src/jit/a64/call.cpp

Same public `JitCall` API as `src/jit/x86/call.cpp` (identical method signatures — verified line-for-line) — this is the arm64 (AAPCS64) implementation of the same class, using asmjit's `a64::Compiler` instead of `x86::Compiler`. Internally follows the same "classify arguments, marshal into native convention, invoke target" shape as the x86 version, adapted to AArch64's register set and argument-passing rules.

### src/jit/a64/callback.cpp

Same public `JitCallback` API as `src/jit/x86/callback.cpp` (identical method signatures — verified line-for-line) — the arm64 reverse-trampoline implementation, same relationship to its x86 counterpart as `call.cpp` above.

### src/jit/a64/helpers.cpp

Implements the same four free functions as the x86 version, arm64-specific:
- `HasHiArgSlot` — unconditionally returns `false` (AArch64 has no 32-bit hi/lo register-splitting case; it's a pure 64-bit-register ABI).
- `GetValueTypeId` / `GetRetTypeId` / `GetCallConvId` — same switch-based mapping approach as x86's helpers.cpp, with arm64-appropriate type/convention mappings.

### src/platform/unix_ops.cpp

**`UnixPlatformOps`** (final class, implements `IPlatformOps` from `include/plugify/platform_ops.hpp`)
- `static int TranslateFlags(LoadFlag)` (private) — maps plugify's `LoadFlag` bitmask to POSIX `dlopen` flag bits (`RTLD_LAZY`/`RTLD_NOW`, `RTLD_GLOBAL`/`RTLD_LOCAL`, optionally `RTLD_NODELETE`/`RTLD_DEEPBIND` where the platform defines them).
- `Result<void*> LoadLibrary(const std::filesystem::path&, LoadFlag) override` — wraps `dlopen`.
- `Result<void> UnloadLibrary(void*) override` — wraps `dlclose`.
- `Result<Address> GetSymbol(void*, std::string_view name) override` — wraps `dlsym`, checking `dlerror()` for failure (since a null symbol can be a valid result).
- `Result<std::filesystem::path> GetLibraryPath(void*) override` — wraps `dladdr` to recover the loaded library's file path from its handle.
- `bool SupportsRuntimePathModification() const override` — returns `false` (no `AddSearchPath`/`RemoveSearchPath` support on this backend).
- `bool SupportsLazyBinding() const override` — returns `true`.

**Free function**
- `std::shared_ptr<IPlatformOps> CreatePlatformOps()` — factory returning a `UnixPlatformOps` instance; this is the file's actual entry point, selected at build time on Unix-like platforms.

### src/platform/windows_ops.cpp

**`WindowsPlatformOps`** (final class, implements `IPlatformOps`) — same interface as `UnixPlatformOps`, backed by Win32 instead of POSIX:
- `static int TranslateFlags(LoadFlag)` (private) — Windows analogue of the Unix version (maps to whatever `LoadLibraryEx` flags Windows supports for the equivalent semantics).
- `static std::string GetLastErrorString()` (private) — formats `GetLastError()` into a human-readable string for error results.
- `Result<void*> LoadLibrary(const std::filesystem::path&, LoadFlag) override` — wraps `LoadLibraryExW`/`LoadLibrary`.
- `Result<void> UnloadLibrary(void*) override` — wraps `FreeLibrary`.
- `Result<Address> GetSymbol(void*, std::string_view) override` — wraps `GetProcAddress`.
- `Result<std::filesystem::path> GetLibraryPath(void*) override` — wraps `GetModuleFileName`.
- `bool SupportsRuntimePathModification() const override` — returns `true` here (unlike Unix).
- `bool SupportsLazyBinding() const override` — returns `false` here (unlike Unix) — Windows has no lazy-binding equivalent to `RTLD_LAZY`.
- `Result<void> AddSearchPath(const std::filesystem::path&) override` — Windows-only capability (`IPlatformOps` method the Unix backend doesn't support): extends the DLL search path.
- `Result<void> RemoveSearchPath(const std::filesystem::path&) override` — reverses `AddSearchPath`.

**Free function**
- `std::shared_ptr<IPlatformOps> CreatePlatformOps()` — factory returning a `WindowsPlatformOps` instance; the Windows-build counterpart of unix_ops.cpp's factory.

### src/pch.hpp

(precompiled header — standard library includes only, no declarations)
