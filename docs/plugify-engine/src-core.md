# plugify `src/core/` — implementation notes

This documents the implementation layer of plugify's plugin engine
(`src/core/`, github.com/untrustedmodders/plugify, MIT licensed). It is a
prose explanation of how the pieces fit together and why, written for
reference — not reproduced source. Small type/function names are quoted
where useful; algorithms and full bodies are described in plain language
instead of pasted.

Every public-facing class declared under `include/plugify/` (e.g. `Extension`,
`Method`, `Manifest`) follows the same shape here: a thin public class holding
a single `std::unique_ptr<Impl>`, with the real fields living in a paired
`*_impl.hpp` file under `src/core/`. This is the PIMPL (pointer-to-implementation)
pattern — every getter/setter on the public class just forwards to `_impl->field`,
copy construction deep-copies the `Impl`, and move construction is the compiler
default (moving the pointer). The point of doing this everywhere is ABI/compile-firewall
stability: consumers linking against the public headers never see the actual field
layout, so adding or reordering fields inside an `Impl` struct doesn't force
recompilation of every translation unit that merely uses the public getters — only
`src/core/` itself, which is the one place that includes the `_impl.hpp` pair.

## Manager (`manager.cpp`) and Plugify (`plugify.cpp`)

**`Manager::Impl`** is the actual engine loop. `Initialize()` discovers every
extension manifest under the configured extensions directory (a breadth-first
directory walk that stops descending into a subdirectory the moment it finds a
manifest file in it, on the assumption that a directory containing a manifest
*is* one extension and its subfolders are private to it), then runs all
discovered extensions through a five-stage `Pipeline<Extension>` (parsing,
dependency resolution, loading, method export, starting — each stage is a
separate class described below). After the pipeline runs, it optionally logs
a human-readable report (load order, dependency graph as text or as Graphviz
DOT) depending on config flags. `Update()` walks every `Running` extension
once per frame and forwards to its language module's update hook. `Terminate()`
walks extensions in **reverse** load order twice — first sending every plugin
an "end" notification, then unloading everything — so dependents always shut
down before their dependencies.

**`Plugify::Impl`** sits one layer above `Manager`: it owns the `ServiceLocator`
(the DI container, described below), validates and creates the configured
directories on startup, and forwards `Update()`/`Terminate()` to the `Manager`
it owns. It also enforces that `Initialize()`/`Update()`/`Terminate()` are all
called from the same thread that constructed it (checked via a stored
`std::thread::id`), since nothing here is meant to be called concurrently
from multiple threads.

**`PlugifyBuilder`** is a fluent builder (`WithBaseDir()`, `WithConfigFile()`,
`WithLogger()`, ...) that resolves a final `Config` by layering, in priority
order: a config file if given, an explicitly-provided base `Config`, then
explicit path/service overrides — each layer only overwrites fields the layer
above actually set (`Config::MergeFrom`, described below). `WithDefaults()`
is what registers the concrete default service implementations — console
logger, platform ops, extended filesystem, `BasicAssemblyLoader`, and the
`LibsolvDependencyResolver` — into the DI container, but only for any service
slot the caller hasn't already filled in themselves.

## Object model: Extension, Binding, Class, Method, Property, Alias, Enum, Value

**`Extension` (`extension.cpp`)** represents one discovered module or plugin.
Its `Impl` tracks identity (id/type/state), the parsed `Manifest`, runtime
state (a `MethodTable` of language-module callback presence flags, an opaque
`userData` pointer the language module gets to stash whatever it wants in),
per-state timing (a map from `ExtensionState` to how long that state took, so
`GetPerformanceReport()` can print a breakdown), and accumulated
errors/warnings. `SetState()` asserts the transition is legal against an
explicit state-machine table (`IsValidTransition`) encoding the whole
lifecycle graph — e.g. `Discovered → Parsing → {Parsed, Failed}`,
`Resolved → Loading → {Loaded, Failed}`, and so on through to
`Terminating → Terminated`. A constructed `Registrar` (see below) is what
makes `ToString(UniqueId)` able to print a human name for debugging anywhere
in the codebase, given only the id.

**`ExtensionLoader` (`extension_loader.hpp`)** is the thing that actually
calls into a language module's C ABI: `LoadModule()` resolves the module's
runtime library via the assembly loader (with an LRU-free cache keyed by
absolute path, so re-requesting the same runtime returns the already-loaded
handle), finds and calls its `GetLanguageModule()` export to get an
`ILanguageModule*`, then calls `Initialize()` on it, passing a `Provider`
(the host-services facade, see below). `LoadPlugin()` similarly calls the
owning module's `OnPluginLoad()`, then validates that the method list the
language module reports back matches the manifest's declared methods
exactly (by pointer identity into the manifest's own method array, not by
re-parsing) before accepting it. Every call into a language module goes
through a `SafeCall` helper that catches `std::bad_alloc`/`std::exception`/
anything else and turns it into a `Result` error instead of letting an
exception cross the extension boundary uncaught.

**`Binding`, `Class`, `Method`, `Property`, `Alias`, `Enum`, `Value`** are all
thin PIMPL wrappers as described above; most are little more than a struct of
optional fields behind getters/setters. Two are worth calling out:

- **`Property`**'s `Impl` holds `prototype`/`enumerate` fields typed as
  `Definition<T> = std::variant<std::shared_ptr<T>, std::string>` — a
  property's function-pointer or enum type can be written in a manifest
  either as a full inline definition or as the *name* of one declared
  elsewhere, and this variant represents "either, not yet resolved" until
  `Manifest::Resolve()` (below) replaces every name with the actual shared
  definition object.
- **`Method`**'s parameter list is a `plg::inplace_vector<Property, 32>` —
  a fixed-capacity, non-heap-allocating vector (from the already-vendored
  `plg/` utility headers) — capping any one function signature at 32
  parameters, chosen so building up a method's signature never triggers a
  heap allocation.

## Dependency resolution and manifest handling

**`Config` (`config.cpp`)** implements layered merging: each config section
(paths/loading/security/logging) tracks which `ConfigSource` (File/Builder/
Override) last wrote it, and `MergeFrom()` only lets a new value overwrite an
existing one if the new source has equal-or-higher priority — this is what
lets `PlugifyBuilder` apply file config, then base config, then explicit
overrides, without a later lower-priority layer clobbering an earlier
higher-priority one. `Validate()` checks the five configured directories
(extensions/configs/data/logs/cache) are pairwise distinct, since two
identical directories would make the loader's discovery walk double-count.

**`Manifest::Resolve()` (`manifest.cpp`)** is the most intricate file in this
directory. A manifest can declare `prototypes`/`enums` up front by name, and
also define them *inline* wherever a property references one — the same
named prototype might legitimately appear written out in full in two
different methods. `Resolve()` runs a two-pass process: pass one walks every
method (and recursively into every prototype it finds, since a prototype's
own parameters can themselves reference further prototypes/enums) and
registers each inline definition into a name-keyed table, collapsing a
repeated definition under an already-seen name onto the first one it saw
(comparing them structurally to error out if the same name is used for two
genuinely different definitions). Pass two then walks everything again,
replacing every by-name reference with the actual object now that every name
in the manifest is known — since pass one guarantees every name is
registered before pass two runs, a reference is free to point either
backward or forward in the manifest text. A separate depth-first walk
(`DetectCycles`) rejects a prototype that can reach itself through nested
prototype parameters, since that would make the resulting `shared_ptr` graph
self-referential (never freed) and would send anything that recursively
walks a signature into unbounded recursion. `Manifest::Validate()` then
checks everything a JSON Schema validator structurally cannot — name
uniqueness across arrays, and cross-field constraints like "a handleless
class can't declare constructors" — on the assumption the manifest already
passed schema validation (see the parsing stage below) so per-field
type/shape checking has already happened elsewhere.

**`LibsolvDependencyResolver` (`libsolv_dependency_resolver.cpp/.hpp`)**
implements `IDependencyResolver` on top of libsolv, the SAT-based package
resolver originally built for Linux distro package managers. The problem it
solves — "given a set of packages each with required/optional/conflicting
dependency constraints, find a consistent installable set and a valid
install order, or explain precisely why none exists" — is exactly the
problem of ordering plugin loading by declared dependencies, so this class
adapts every discovered `Extension` into libsolv's data model instead of
writing a bespoke resolver: each extension becomes a libsolv "solvable" in
an in-memory "pool" (`InitializePool`/`AddSolvable`), each manifest
dependency/conflict/obsolete constraint becomes a libsolv dependency
relation (`REQUIRES`/`RECOMMENDS`/`CONFLICTS`/`OBSOLETES`, built via
`pool_rel2id` with version-comparison operators translated from plugify's
own `Constraint`/range-operator types into libsolv's `REL_LT`/`REL_EQ`/etc.
flags). `RunSolver()` then asks libsolv's SAT solver to find an installation
covering every extension; if the solver reports conflicts, each problem is
translated back into plugify's own `DependencyIssue` structure (including
libsolv's own suggested fixes, extracted via its solution-enumeration API).
On success, libsolv's transaction object already contains a valid
topological install order, which `ComputeInstallationOrder()` walks to build
plugify's own load-order list and both directions of the dependency graph
(by cross-referencing each solvable's `REQUIRES` list against libsolv's
"what provides this" index). Every libsolv object with manual lifetime
(`Pool`, `Solver`, `Transaction`, `Queue`) is wrapped in a `unique_ptr` with a
custom deleter calling the matching `*_free()` C function, so nothing here
needs a manual cleanup path even on an early return.

**`glaze_metadata.hpp`** is what makes plugify's own PIMPL classes (`Method`,
`Manifest`, `Dependency`, `Property`, etc.) parseable as JSON by glaze: since
glaze's reflection normally works on plain member pointers, and these classes
hide their real fields behind `_impl`, each type gets an explicit
`glz::meta<T>` specialization whose "member pointers" are actually lambdas
reaching through `self._impl->field` — this is how glaze is told to read/write
the field inside the `Impl` struct as if it were a normal public member, without
those classes actually exposing their internals as a real public API. Fields
that exist in the JSON manifest format purely for human documentation
(`description`, `deprecated`, `group`) are marked `skip{}` so glaze ignores
them on both read and write.

**`glaze_adapter.hpp`** bridges the other direction: it implements valijson's
`Adapter` interface (the interface valijson uses to walk *any* JSON-like
document type when validating it against a schema) on top of glaze's own
generic/DOM JSON type (`glz::generic`, or `glz::json_t` on older glaze
releases — detected at compile time via `__has_include`, since the type was
renamed and its backing container changed between glaze major versions, and
the file is written entirely against the common subset of API both versions
share so no version-specific logic is needed elsewhere). This is what lets
plugify validate a raw parsed manifest document against a JSON Schema (via
valijson) before ever deserializing it into `Manifest` — schema validation
and glaze's typed deserialization are two separate passes over two different
representations of the same JSON text, joined by this adapter. One notable
detail: glaze's generic DOM stores every JSON number as a plain `double`
(no separate integer storage), so the adapter classifies a value as
"integer" for schema-checking purposes if it has no fractional part and fits
in an `int64_t`, mirroring how JSON Schema itself defines its "integer" type.

**`Provider`/`Registrar`/`ServiceLocator`** round out the supporting cast:
`Provider` is the read-only facade a language module receives during
`Initialize()` — logging, config directory getters, and read-only extension
queries, all forwarding to the `Manager`/`Config`/`ServiceLocator` it was
built from. `Registrar` is a small RAII object that inserts a
`UniqueId → name` pair into a process-wide table (guarded by a
`shared_mutex`) on construction and removes it on destruction, purely so
`ToString(UniqueId)` can print a name anywhere in the codebase without
threading a name parameter through every function that only has an id.
`ServiceLocator` is a small dependency-injection container keyed by
`std::type_index`, supporting singleton/transient/scoped lifetimes (scoped
services live in a thread-local stack of scope maps, pushed/popped by
`ScopedServiceLocator`'s constructor/destructor) — this is what lets
`PlugifyBuilder` register a custom logger/filesystem/assembly-loader and have
every consumer resolve the same instance without a hard compile-time
dependency between them.

## Pipeline machinery (`pipeline.hpp`, `stages.hpp`, `stages_impl.hpp`)

**`stages.hpp`** defines three abstract stage shapes a `Pipeline<T>` can run:
`ITransformStage` (processes every item independently and in parallel — no
reordering), `IBarrierStage` (gets the whole container at once and may
reorder/filter/drop items — used exactly once, by dependency resolution,
since that's the one stage whose output order matters), and `ISequentialStage`
(processes items one at a time in container order, optionally continuing
past a per-item failure).

**`Pipeline<T>` (`pipeline.hpp`)** is a small builder-configured stage
runner: `Create().AddStage(...).AddStage(...)...Build()` produces an
immutable ordered list of stages, and `Execute()` runs each stage over the
item container in turn, stopping early if a *required* stage reports any
failures. `ITransformStage` items are farmed out across a thread pool
(glaze's own `glz::pool`, reused here purely as a lightweight thread pool
utility, unrelated to glaze's JSON functionality) and joined before the next
stage starts. A `Report`/`StageStatistics` pair accumulates per-stage
in/out item counts, success/failure counts, elapsed time, and per-item error
messages, which is what backs the human-readable pipeline summary logged
after `Manager::Initialize()`.

**`stages_impl.hpp`** is where the five concrete stages actually live:
- `ParsingStage` (transform): loads each extension's manifest file, validates
  the raw JSON against a compiled `valijson::Schema` (one schema per
  extension type, loaded once in `Setup()`), deserializes it into a
  `Manifest` via glaze, then calls `Resolve()` then `Validate()` on it.
- `ResolutionStage` (barrier): filters extensions by whitelist/blacklist/
  platform-support policy first, injects an implicit dependency from every
  plugin onto its declared language's module (so a Python plugin always
  depends on the Python language module, even though the manifest never says
  so explicitly), then hands the whole filtered set to the configured
  `IDependencyResolver` and rebuilds the container in the resolver's load
  order, appending unresolved/excluded extensions at the end (with their
  reasons recorded as errors/warnings on each `Extension`).
- `LoadingStage`, `ExportingStage`, `StartingStage` (sequential) all inherit
  from a shared CRTP base, `BaseFailurePropagatingStage<Derived>`, which
  factors out one recurring rule: before processing an item, check whether
  any of its dependencies already failed (via the reverse-dependency graph
  built by the resolution stage), and if so mark it `Skipped` without
  attempting to process it at all; and after a real failure, propagate that
  failure forward to the item's own dependents so the next stage's check
  catches them too. Each derived stage only implements the actual
  operation-specific `DoProcessItem()` (calling `LoadModule`/`LoadPlugin`,
  `MethodExport`, or `StartPlugin` on the `ExtensionLoader`) — the shared
  dependency-failure bookkeeping lives once, in the base.

## Logging, filesystem, and assembly loading

**`ConsoleLogger`/`FileLogger` (`console_logger.hpp`/`file_logger.hpp`)**
implement `ILogger`. `ConsoleLogger` writes timestamped, source-location-
tagged lines to stdout/stderr (severity-gated by a settable minimum level).
`FileLogger` inherits from it, additionally writing every line to a
timestamped log file and rotating to a new timestamped file once the current
one crosses a configurable size threshold, while still echoing
warning-and-above lines to the console via the base class's formatting.

**`FailureTracker` (`failure_tracker.hpp`)** is the small shared piece of
state the loading/exporting/starting stages all consult: a
`shared_mutex`-guarded set of failed extension ids, with helpers to check
whether any of a given extension's dependencies are in that set (using the
reverse-dependency graph) and to name the specific failed dependency for an
error message.

**`AssemblyHandle`/`BasicAssembly`/`BasicAssemblyLoader`
(`assembly_handle.hpp`, `basic_assembly.hpp`, `basic_assembly_loader.hpp`)**
form the actual native-library-loading stack. `AssemblyHandle` is a small
move-only RAII wrapper around a raw OS library handle plus the
platform-abstraction object (`IPlatformOps`) needed to unload it, so the
handle is guaranteed released exactly once. `BasicAssembly` implements the
public `IAssembly` interface as a thin forward onto an `AssemblyHandle`.
`BasicAssemblyLoader` implements `IAssemblyLoader::Load()`: it resolves the
requested path to an absolute one, checks a `weak_ptr`-valued cache first (so
a library already loaded and still in use is returned rather than reloaded,
but a cache entry whose `IAssembly` has since been destroyed is treated as a
miss), otherwise adds any extra runtime search paths the platform supports
(removed again via a scope guard once loading finishes, success or not),
loads the library through `IPlatformOps::LoadLibrary()`, and wraps the
result. `Unload()` just removes the cache entry.

**`StandardFileSystem`/`ExtendedFileSystem` (`standart_file_system.cpp/.hpp`)**
implement `IFileSystem` directly over `std::filesystem` and `<fstream>` —
read/write text and binary files, directory listing/iteration (including a
recursive variant with an optional regex filename filter and predicate
filter), and path utilities (absolute/canonical/relative), every operation
returning a `Result<T>` with a message assembled from the platform's actual
error (`errno`/`strerror_r` on POSIX, `GetLastError`/`FormatMessageA` on
Windows) rather than a generic failure string. `ExtendedFileSystem` adds
append operations, an atomic write (write to a temp file, then rename over
the target), temp file/directory creation, simple content/hash comparison,
and permission/symlink utilities layered on top.
