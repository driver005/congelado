# plugify's `include/plugify/` — architecture notes

This documents [plugify](https://github.com/untrustedmodders/plugify) (MIT license,
untrustedmodders), a C++ plugin-loading engine congelado is evaluating ideas from. Everything
below is a prose explanation of how the public headers in `include/plugify/` fit together,
written for reference — not reproduced source. Small type/method names are quoted where useful
to anchor the explanation, but no function bodies or large code blocks are copied here.

Almost every "domain object" class in this directory (`Dependency`, `Conflict`, `Method`,
`Class`, `Property`, `Value`, `Enum`, `Binding`, `Alias`, `Extension`, `Manager`, `Provider`,
`ServiceLocator`, `PlugifyBuilder`, `Plugify`) follows the same shape: a small public class with
getters/setters and comparison operators, backed by a private `struct Impl` and a
`std::unique_ptr<Impl>` member (the PIMPL pattern). The real fields only exist in the `.cpp`
file's `Impl` definition — the public header never exposes layout, so plugify's own shared
library can change its internals without breaking binary compatibility with code that only
includes the header. `global.h`'s `PLUGIFY_ACCESS` macro controls whether that `Impl` type is
`public` or `private` depending on whether the *core* library itself is being compiled
(`PLUGIFY_CORE`) or an external consumer is just including the header.

## Foundational types

### `value_type.hpp`
Defines `ValueType`, a single enum covering every type plugify can carry across a plugin
boundary: void/bool/chars/signed & unsigned ints of every width/pointer/float/double, plus
higher-level kinds — `Function`, `String`, `Any`, a full parallel set of `Array*` variants for
each of those, and small vector/matrix struct types (`Vector2/3/4`, `Matrix4x4`). Alongside the
enum, `ValueUtils` is a static-method toolbox for reasoning about a `ValueType` value: `IsScalar`,
`IsFloating`, `IsObject`, `IsArray`, `IsStruct`, and critically `IsHiddenParam` — this predicate
decides whether a given return type is big/non-trivial enough that it must be returned via a
caller-allocated hidden pointer argument rather than in registers, which is exactly the ABI
convention the JIT call-generation code (outside this directory, in `src/jit/`) has to honor.
`ValueUtils::SizeOf` maps each `ValueType` to the `sizeof` of its concrete C++ representation
(most of which are the `plg::` container types congelado already vendored). A block of
`static_assert`s at the bottom locks the enum's ordering to `plg::any`'s variant-index ordering,
so the two stay in sync by construction — if someone reorders one without the other, the build
fails immediately rather than corrupting data silently at runtime.

### `load_flag.hpp`
A tiny bitflag enum (`LoadFlag`) for how a shared library should be `dlopen`'d: lazy symbol
binding, global symbol visibility, prevent-unload, deep-bind (Linux), secure search paths
(Windows). Uses `plg::bitmask`'s `enable_bitmask_operators` hook (from the already-vendored
`plg/` library) so the enum supports `|`/`&` like a real flag set.

### `lifecycle.hpp`
One abstract interface, `IExtensionLifecycle`, with hook methods (`OnLoad`, `OnUnload`,
`OnStart`, `OnEnd`, `OnUpdate`, `OnExport`) that fire as an `Extension` moves through its life.
This is a customization point — `PlugifyBuilder::WithExtensionLifecycle` lets a host application
supply its own implementation to observe or react to those transitions.

### `types.hpp`
Grab-bag of small shared aliases and helpers used everywhere else: `Version` (aliases
`plg::version`), `Constraint` (a version-range set, for dependency requirements like "needs
libX ≥1.2 <2.0"), `Location` (aliases `plg::source_location`), and `Result<T>` — an alias for
`std::expected<T, std::string>` used as the return type of almost every fallible operation in
the engine instead of exceptions. `UniqueId` is a strongly-typed wrapper around a `ptrdiff_t`
identifier (every `Extension` gets one), with a debug-only name field attached purely for
readability in a debugger, not used in any actual logic. `MakeError` is a couple of overloads
for constructing a `Result`'s error case, either from a plain string or a `std::format` format
string.

## Manifest, dependency & conflict descriptors

### `dependency.hpp`
`Dependency` — a name, a `Constraint` (version range), and an "is this optional" flag. Describes
one entry in an extension's `dependencies` list.

### `conflict.hpp`
`Conflict` — a name, a `Constraint`, and a human-readable reason string, describing an extension
this one is incompatible with. `Obsolete` is just an alias for the same `Conflict` type, reused
to describe extensions this one replaces/supersedes.

### `dependency_resolver.hpp`
Defines the actual dependency-resolution *interface* (`IDependencyResolver`) that plugify's
`libsolv`-backed resolver (in `src/core/`, outside this directory) implements — congelado could
substitute its own implementation here instead. `Resolve(extensions)` takes every known
`Extension` and returns a `ResolutionReport`: a computed load order, a full dependency graph and
its reverse (for figuring out what breaks if one extension is removed), and a list of
`DependencyIssue`s (which extension, which problem, whether it's blocking, and optional
suggested fixes) for anything that didn't resolve cleanly.

### `manifest.hpp`
`Manifest` is a plain aggregate struct (not PIMPL'd — everything here is intentionally visible)
representing everything an extension's manifest file can declare: common metadata (name,
version, author, license…), its `dependencies`/`conflicts`/`obsoletes`, plugin-only fields
(`entry` point, exported `methods`, `classes`), module-only fields (`runtime`, extra search
`directories`), and two "shared type table" lists — `prototypes` and `enums` — that let a
manifest define a method signature or enum once and have multiple `Property` entries reference
it by name instead of repeating the definition. `Resolve()` is the pass that links those
by-name references to their definitions and folds any inline definitions into those shared
tables so every distinct type appears exactly once afterward; `Validate()` checks the whole
thing is internally consistent. Both return `Result<...>` rather than throwing.

## Core plugin-description model

### `enum.hpp` / `value.hpp` (the `Enum`/`Value` pair)
`Value` is a tiny PIMPL class: a name plus an `int64_t` value — one member of a user-defined
enum a plugin's manifest declares. `Enum` groups a name with a list of those `Value`s. These are
what `Property::GetEnumerate()` (below) points at when a parameter's type is a plugin-defined
enum rather than a built-in `ValueType`.

### `property.hpp`
`Property` describes one function parameter or return value: its `ValueType`, whether it's
passed `IsRef` (by reference), and — only when relevant — a `Prototype` (for `Function`-typed
values, describing the callback's own signature) or an `Enum` (for enum-typed values).

### `method.hpp`
`Method` is a full function signature: a fixed-capacity list of parameter `Property`s (capped at
`Signature::kMaxFuncArgs`, from `signarure.hpp`), a return `Property`, its declared/exported
name, the raw C symbol name to bind to, a `CallConv`, and a variadic-argument index if
applicable. `Prototype` is just an alias for `Method`, reused wherever a `Function`-typed
`Property` needs to describe its own signature. `MethodData` pairs a `Method` reference with the
resolved `Address` of its actual symbol once a plugin is loaded — this is the bridge between the
static manifest description and the real, loaded function pointer. `MethodTable` is a small
struct of booleans recording which of the well-known lifecycle entry points (update/start/end/
export) a loaded plugin actually exports, so the host doesn't have to probe for symbols that
don't exist every frame.

### `class.hpp` / `binding.hpp` / `alias.hpp`
`Class` describes an opaque "handle" type a plugin exposes to other languages — a name, the
underlying `ValueType` used to represent its handle (usually a pointer), a sentinel
"invalid value" string, its constructor/destructor symbol names, and a list of `Binding`s.
`Binding` maps one native method onto the class as a bound member (optionally "bind self" — i.e.
implicitly pass the handle as an argument), with per-parameter and per-return `Alias`
information. `Alias` (a name plus an "is owner" flag) exists so generated language bindings know
whether a parameter/return value transfers ownership of the underlying resource or is just a
borrowed reference — relevant for memory management in whatever target language a binding is
generated for.

### `extension.hpp`
`Extension` is the big unified runtime object: every module and plugin plugify tracks is one of
these, distinguished by `GetType()` (`ExtensionType::Module` or `::Plugin`). It carries an
`ExtensionState` (a long linear state machine from `Discovered` through `Parsing`/`Parsed`,
`Resolving`/`Resolved`, `Loading`/`Loaded`, `Starting`/`Started`, `Running`, all the way to
`Terminated`, with `Corrupted`/`Unresolved`/`Failed`/`Skipped` as off-ramps) plus timing data per
state transition (`StartOperation`/`EndOperation`, `GetOperationTime`, `GetPerformanceReport`).
It re-exposes everything from its `Manifest` as direct getters, holds the resolved `Address` of
plugin-specific user data and its `MethodTable`, a pointer to the `ILanguageModule` that owns it
(see below), and — for modules specifically — the loaded `IAssembly`. `AddError`/`AddWarning`
accumulate diagnostics without failing outright; `IsValidTransition` is a static helper that
checks whether one `ExtensionState` can legally follow another, keeping the state machine
enforced centrally rather than scattered across call sites.

### `language_module.hpp`
`ILanguageModule` is the interface a *module* (as opposed to a plugin) implements — this is the
plugin-in-a-different-language bridge concept: a Python module, a Lua module, etc. all implement
this interface so plugify's core can hand plugin-load/start/update/end events to whichever
runtime is actually hosting that plugin's code, without the core needing to know anything about
Python or Lua specifically. `Initialize`/`Shutdown` bracket the module's whole lifetime;
`OnPluginLoad` returns a `LoadData` (the plugin's resolved exported `MethodData` list, its user
data `Address`, and its `MethodTable`) that becomes the corresponding `Extension`'s runtime
state.

## Manager, provider & top-level `Plugify`/`PlugifyBuilder`

### `manager.hpp`
`Manager` owns the whole extension registry: `Initialize`/`Update`/`Terminate` the collection,
look extensions up by name or `UniqueId` (`FindExtension`, with an optional version
`Constraint`), filter by state or type, and dump diagnostics (`GenerateLoadOrder`,
`GenerateDependencyGraph`/`...GraphDOT` for Graphviz output). Extension load/unload/reload
methods exist in the header only as commented-out stubs — not yet part of the public API in this
version.

### `provider.hpp`
`Provider` is a facade handed to language modules/plugins so they don't need direct access to
the full `ServiceLocator` — it re-exposes logging (with per-severity convenience wrappers:
`LogTrace`/`LogDebug`/`LogInfo`/`LogWarning`/`LogError`/`LogFatal`, all funneling into one
virtual `Log` call), the configured path directories (extensions/configs/data/logs/cache),
extension lookup, and generic `Resolve<Service>()`/`TryResolve<Service>()` access to whatever's
registered in the underlying `ServiceLocator`.

### `service_locator.hpp`
A small dependency-injection container. Services are registered by interface type
(`std::type_index`) either as a concrete singleton instance, a factory function, or an
auto-constructed type, each with a `ServiceLifetime` (`Transient`/`Scoped`/`Singleton`).
`RegisterWithDependencies` supports constructor-injection-style wiring — a factory that itself
calls `Resolve<Dependency>()` for each constructor argument. `ServiceLocator::ServiceBuilder` is
a small chained fluent-API wrapper (`.AddSingleton<T>().AddFactory<U>(...)`) over the same
registration methods. `ScopedServiceLocator` is a plain RAII guard around `BeginScope`/`EndScope`
for scoped-lifetime services.

### `plugify.hpp`
Ties everything together. `PlugifyBuilder` is the entry point: chain `WithBaseDir`/`WithConfig`/
`WithLogger`/`WithFileSystem`/`WithAssemblyLoader`/`WithDependencyResolver`/
`WithExtensionLifecycle`/generic `WithService<Interface, Impl>()` calls, then call `Build()` to
get a `Result<std::shared_ptr<Plugify>>` — every one of those `With*` calls is really just
registering an implementation into the underlying `ServiceLocator` before construction.
`WithDefaults()` presumably fills in the platform-default implementations for whatever wasn't
explicitly overridden. `Plugify` itself is the resulting facade object: `Initialize`/`Terminate`/
`Update`, and accessors for the `Manager`, the `ServiceLocator`, the resolved `Config`, and the
engine's own `Version`. `MakePlugify(rootDir)` is a one-call convenience wrapper around
`CreateBuilder().WithBaseDir(rootDir).WithDefaults().Build()` for the common case.

## Remaining infrastructure/utility headers

### `logger.hpp`
`Severity` (Trace through Fatal) plus `ILogger`, a minimal logging interface (`Log`,
`SetLogLevel`/`GetLogLevel`, `Flush`) that `Provider`'s logging helpers and the engine's own
internals log through — a host application supplies its own implementation via
`PlugifyBuilder::WithLogger`.

### `profiler.hpp`
`IProfiler` is an optional CPU-profiling hook: `MarkFrame` for frame boundaries, paired
`BeginZone`/`EndZone` calls (each returning/taking a `ZoneHandle`) for timed regions.
`ScopedZone` is the RAII wrapper meant to be used instead of calling `BeginZone`/`EndZone`
directly — it begins a zone in its constructor and ends it in its destructor (or on early
move-out), so a profiled region can't be left unbalanced by an early return or exception.

### `service_locator.hpp` / `registrar.hpp`
`registrar.hpp` is small: `Registrar` is an RAII helper that registers a `UniqueId`+name pair on
construction and presumably unregisters it on destruction (in some process-wide registry
implemented in the `.cpp`), plus a free `ToString(UniqueId)` helper for turning an id back into
its debug name.

### `global.h`
Two macros: `PLUGIFY_ACCESS` (public inside the core library build, private for external
consumers — this is what makes every class's `Impl`/`_impl` PIMPL member effectively invisible
outside the library while still letting the library's own `.cpp` files see it), and
`PLUGIFY_NO_DLL_EXPORT_WARNING`, which just wraps a declaration to suppress MSVC's C4251
("needs to have dll-interface") warning on non-exported members like a `unique_ptr<Impl>`.

### `alias.hpp` / `binding.hpp` / `callback.hpp` / `call.hpp` / `address.hpp` / `signarure.hpp`
Already covered `alias.hpp`/`binding.hpp` above alongside `class.hpp`, and `signarure.hpp`
(`CallConv`, `Signature`) alongside `method.hpp`. `address.hpp` defines `Address`, a
zero-overhead wrapper around a raw `uintptr_t` with pointer-arithmetic-style operators
(`Offset`, `Deref`, `+`/`-`/bitwise ops) and safe conversions to/from real pointers — used
everywhere in this directory instead of a bare `void*` so memory addresses have a consistent,
slightly safer vocabulary type. `callback.hpp` (`JitCallback`) and `call.hpp` (`JitCall`) are
the *interfaces* to the actual JIT machinery that lives in `src/jit/` (outside this directory):
`JitCallback::GetJitFunc` generates a native function pointer that, when called by arbitrary
native code, forwards into a `CallbackHandler` — this is how a plugin can hand out a real C
function pointer that's backed by managed/scripted code. `JitCall::GetJitFunc` is the inverse:
given a target function `Address` and a `Signature`, it generates a trampoline that lets code
call that target dynamically without knowing its signature at compile time. Both defer to
`ValueUtils::IsHiddenParam` by default to decide whether a return value needs the
caller-allocates-and-passes-a-hidden-pointer convention. `ParametersSpan`/`ReturnSlot` (in
`callback.hpp`) and `Parameters`/`Return` (in `call.hpp`) are the small helper types both JIT
paths use to read/write register-sized argument slots and return-value storage without directly
juggling raw byte offsets at each call site.

### `assembly.hpp` / `assembly_loader.hpp` / `file_system.hpp` / `platform_ops.hpp`
`IAssembly` is the loaded-shared-library abstraction (symbol lookup, base address, raw platform
handle); `IAssemblyLoader` is the interface that actually performs `Load`/`Unload` given a path
and `LoadFlag`s, returning a `Result<AssemblyPtr>`. `IFileSystem` is a full filesystem
abstraction (read/write text or binary files, directory listing/iteration with filters, path
resolution) — supplied via `PlugifyBuilder::WithFileSystem`, presumably so tests or sandboxed
environments can substitute an in-memory filesystem. `IPlatformOps` is the lowest-level OS
abstraction underneath the assembly loader: raw `LoadLibrary`/`UnloadLibrary`/`GetSymbol` calls,
capability queries (`SupportsRuntimePathModification`, `SupportsLazyBinding`), and optional
runtime search-path management — `CreatePlatformOps()` is a factory function implemented once
per platform (this is the header whose real implementations, `unix_ops.cpp`/`windows_ops.cpp`
— plus the PlayStation/Switch variants congelado is dropping — live in `src/platform/`).

### `config.hpp`
`Config` is the big plain aggregate covering everything `PlugifyBuilder` can configure: `Paths`
(base/extensions/configs/data/logs/cache directories, each with a `HasCustom*Dir()` check and a
`ResolveRelativePaths()` pass that makes them absolute against `baseDir`), `Loading` (whether to
prefer a plugin's own symbols over global ones, max concurrent loads, per-phase timeouts),
`Security` (extension whitelist/blacklist, excluded directories), and `Logging` (verbosity,
several print-report/print-load-order/print-dependency-graph flags, and an optional DOT-export
path). Every sub-struct tracks whether its fields came from a default, a config file, a builder
call, or an explicit override (`SourceTracking`/`ConfigSource`) so `MergeFrom` can combine
several partial configs (e.g. defaults + file + builder overrides) with correct precedence
without silently clobbering an explicitly-set value with a lower-priority default.
