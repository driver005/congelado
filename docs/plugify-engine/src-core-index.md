# src/core/ — structured class/function index

Structured class/function index for plugify's `src/core/` (MIT license, github.com/untrustedmodders/plugify) — public signatures and one-line purposes only, companion to `src-core.md`'s prose explanations.

For classes with substantial internal algorithms (dependency/manifest resolution, the pipeline execution engine, the service-locator DI container), only the **public interface** is listed here — internal helper methods and the mechanism behind them are covered in prose in `src-core.md`, not itemized here, since enumerating that structure in signature form would just re-encode the same design in a different shape.

For the ~15 data-holder classes below (PIMPL wrappers around a manifest field with getters/setters), every method is listed — they're structurally identical, formulaic accessor patterns with no meaningful variation between them.

---

## Top-level orchestration

### `manager.cpp` — `Manager`
- `Manager(const ServiceLocator&, const Config&)` — constructs, wiring up services from the locator.
- `~Manager()`
- `Result<void> Initialize() const` — discovers extensions, runs them through the load pipeline.
- `bool IsInitialized() const noexcept`
- `void Update(std::chrono::milliseconds) const` — advances all running extensions one tick.
- `void Terminate() const` — unwinds all running extensions in reverse order.
- `bool IsExtensionLoaded(std::string_view name, std::optional<Constraint>) const noexcept`
- `const Extension* FindExtension(std::string_view name) const noexcept`
- `const Extension* FindExtension(UniqueId) const noexcept`
- `std::vector<const Extension*> GetExtensions() const`
- `std::vector<const Extension*> GetExtensionsByState(ExtensionState) const`
- `std::vector<const Extension*> GetExtensionsByType(ExtensionType) const`
- `std::string GenerateLoadOrder() const` / `GenerateDependencyGraph() const` / `GenerateDependencyGraphDOT() const` — debug reports.
- `operator==` / `operator<=>` — defaulted, by internal state.

*(Internal `Manager::Impl` owns extension discovery, a 4-stage load pipeline (parse → resolve → load → export → start), and per-extension teardown ordering — see `src-core.md`.)*

### `plugify.cpp` — `Plugify`, `PlugifyBuilder`
**`Plugify`**
- `Plugify(ServiceLocator, Config)`
- `~Plugify()`
- `Result<void> Initialize() const` — must be called from the owning thread.
- `void Terminate() const`
- `bool IsInitialized() const`
- `void Update(std::chrono::milliseconds) const`
- `const Manager& GetManager() const noexcept`
- `const ServiceLocator& GetServices() const noexcept`
- `const Config& GetConfig() const noexcept`
- `const Version& GetVersion() const noexcept`
- `static PlugifyBuilder CreateBuilder()`

**`PlugifyBuilder`** (fluent builder)
- `PlugifyBuilder()` / `~PlugifyBuilder()`
- `WithBaseDir(std::filesystem::path)`, `WithPaths(Config::Paths)`, `WithConfigFile(std::filesystem::path)`, `WithConfig(Config)` — path/config overrides, each returns `*this`.
- `WithLogger/WithProfiler/WithFileSystem/WithAssemblyLoader/WithDependencyResolver/WithExtensionLifecycle(std::shared_ptr<...>)` — service overrides.
- `WithDefaults()` — fills any unregistered service with its default implementation (`ConsoleLogger`, platform ops, `ExtendedFileSystem`, `BasicAssemblyLoader`, `LibsolvDependencyResolver`).
- `Result<std::shared_ptr<Plugify>> Build()` — merges config (file → builder → overrides, in priority order), validates it, applies defaults, constructs `Plugify`.
- `const ServiceLocator& GetServices() const noexcept`
- `Result<Config> LoadConfigFromFile(const std::filesystem::path&) const`

**Free function**
- `Result<std::shared_ptr<Plugify>> plugify::MakePlugify(const std::filesystem::path& rootDir)` — convenience one-call factory.

---

## Object model

### `extension.cpp` — `Extension`
- `Extension()` / `Extension(UniqueId, std::filesystem::path location)` — the latter derives name/type from the manifest path.
- `~Extension()`, move ctor/assign (no copy).
- `GetId/GetType/GetState/GetName/GetVersion/GetLanguage/GetLocation() const noexcept` — core identity getters.
- `GetDescription/GetAuthor/GetWebsite/GetLicense() const noexcept` — optional manifest fields, empty string if absent.
- `GetPlatforms/GetDependencies/GetConflicts/GetObsoletes() const noexcept`
- `GetEntry/GetMethods/GetClasses/GetPrototypes/GetEnums/GetMethodsData() const noexcept` — plugin-only fields, empty if this is a module.
- `GetRuntime/GetDirectories/GetAssembly() const noexcept` — module-only fields, empty if this is a plugin.
- `GetUserData/GetMethodTable/GetLanguageModule/GetManifest() const noexcept`
- `GetErrors/GetWarnings() const noexcept`, `HasErrors/HasWarnings() const noexcept`
- `GetOperationTime(ExtensionState) const`, `GetTotalTime() const`, `GetPerformanceReport() const` — per-state timing, accumulated during `StartOperation`/`EndOperation`.
- `StartOperation(ExtensionState)` / `EndOperation(ExtensionState)` — bracket a lifecycle transition, recording its duration.
- `SetState(ExtensionState)` — asserts the transition is one `IsValidTransition` allows.
- `AddError/AddWarning(std::string)`, `ClearErrors/ClearWarnings()`
- `SetUserData/SetMethodTable/SetLanguageModule/SetManifest/SetMethodsData/SetAssembly(...)` — runtime setters.
- `operator==` (by id), `operator<=>` (by id)
- `static ExtensionType GetExtensionType(const std::filesystem::path&)` — by file extension (`.pplugin`/`.pmodule`).
- `static bool IsValidTransition(ExtensionState from, ExtensionState to)` — the lifecycle state machine's transition table.
- `std::string ToString() const`
- `AddDependency(std::string)`, `Reset()`

### `extension_loader.hpp` — `ExtensionLoader`
- `ExtensionLoader(const ServiceLocator&, const Config&, const Provider&)`
- `Result<void> LoadModule(Extension&)` / `UnloadModule(Extension&)`
- `Result<void> UpdateModule(Extension&, std::chrono::milliseconds)`
- `Result<void> LoadPlugin(const Extension& module, Extension& plugin)` / `UnloadPlugin(Extension&)`
- `Result<void> StartPlugin(Extension&)` / `EndPlugin(Extension&)` / `UpdatePlugin(Extension&, std::chrono::milliseconds)`
- `Result<void> MethodExport(const Extension& module, Extension& plugin)` — exports a plugin's methods through a module's language binding.
- `const LoadStatistics& GetStatistics() const`, `ResetStatistics()`, `Clear()`

*(Internally: caches loaded assemblies by path, calls into a language module's `ILanguageModule` interface for each lifecycle event, wraps every call through a `SafeCall` helper that turns thrown exceptions into `Result` errors.)*

Also declares `LoadStatistics` (modules/plugins loaded counters, slowest-load tracking, `Summary()`) and a private `ScopedTimer<Callback>` RAII helper that invokes a callback with elapsed time on destruction.

### `binding.cpp`/`binding_impl.hpp` — `Binding`
- `Binding()`, copy/move ctor+assign, `~Binding()`
- `GetName/GetMethod() const noexcept`, `IsBindSelf() const noexcept`
- `GetParamAliases() const noexcept`, `GetRetAlias() const noexcept`
- `SetName/SetMethod(std::string)`, `SetBindSelf(bool)`, `SetParamAliases(...)`, `SetRetAlias(std::optional<Alias>)`
- `operator==`/`operator<=>` (defaulted)

### `class.cpp`/`class_impl.hpp` — `Class`
- `Class()`, copy/move ctor+assign, `~Class()`
- `GetName() const noexcept`, `GetHandleType() const noexcept` (defaults to `ValueType::Void`)
- `GetInvalidValue/GetDestructor() const noexcept`, `GetConstructors() const noexcept`, `GetBindings() const noexcept`
- `SetName/SetInvalidValue/SetDestructor(std::string)`, `SetHandleType(ValueType)`, `SetConstructors(std::vector<std::string>)`, `SetBindings(std::vector<Binding>)`
- `operator==`/`operator<=>` (defaulted)

### `method.cpp`/`method_impl.hpp` — `Method`
- `Method()`, copy/move ctor+assign, `~Method()`
- `GetParamTypes() const noexcept`, `GetRetType() const noexcept`, `GetName/GetFuncName() const noexcept`
- `GetCallConv() const noexcept` (defaults `CDecl`), `GetVarIndex() const noexcept` (defaults `kNoVarArgs`)
- `SetParamTypes(...)`, `SetRetType(Property)`, `SetName/SetFuncName(std::string)`, `SetCallConv(CallConv)`, `SetVarIndex(uint8_t)`
- `operator==`/`operator<=>` (defaulted)

### `property.cpp`/`property_impl.hpp` — `Property`
- `Property()`, copy/move ctor+assign, `~Property()`
- `GetType() const noexcept`, `IsRef() const noexcept`
- `GetPrototype() const noexcept` / `GetEnumerate() const noexcept` — resolve a `Definition<T>` (variant of `shared_ptr<T>` or unresolved name) to its definition, null if still a name.
- `SetType(ValueType)`, `SetRef(bool)`, `SetPrototype(std::shared_ptr<Prototype>)`, `SetEnumerate(std::shared_ptr<Enum>)`
- `operator==`/`operator<=>` (defaulted)
- Free helper: `template<class T> std::shared_ptr<T> DefinitionOf(const Definition<T>&) noexcept`

### `alias.cpp`/`alias_impl.hpp` — `Alias`
- `Alias()`, copy/move ctor+assign, `~Alias()`
- `GetName() const noexcept`, `IsOwner() const noexcept`
- `SetName(std::string)`, `SetOwner(bool)`
- `operator==`/`operator<=>` (defaulted)

### `enum.cpp`/`enum_impl.hpp` — `Enum`
- `Enum()`, copy/move ctor+assign, `~Enum()`
- `GetName() const noexcept`, `GetValues() const noexcept`
- `SetName(std::string)`, `SetValues(std::vector<Value>)`
- `operator==`/`operator<=>` (defaulted)

### `value.cpp`/`value_impl.hpp` — `Value` (an enum member's name+value pair)
- `Value()`, copy/move ctor+assign, `~Value()`
- `GetName() const noexcept`, `GetValue() const noexcept` (`int64_t`)
- `SetName(std::string)`, `SetValue(int64_t)`
- `operator==`/`operator<=>` (defaulted)

---

## Dependency / manifest handling

### `config.cpp` — `Config` (free functions on the struct, no PIMPL)
- `void MergeFrom(const Config& other, ConfigSource)` — merges another config's fields in, respecting per-section source priority (paths/loading/security/logging merged independently).
- `void MergeField(std::string_view fieldPath, const Config&, ConfigSource)` — merges just one named section.
- `Result<void> Validate() const` — checks base dir is set and no two path fields collide.
- `static const std::unordered_set<std::filesystem::path>& GetDefaultExcludedDirs()`

### `conflict.cpp`/`conflict_impl.hpp` — `Conflict`
- `Conflict()`, copy/move ctor+assign, `~Conflict()`
- `GetName() const noexcept`, `GetConstraints() const noexcept`, `GetReason() const noexcept`
- `SetName(std::string)`, `SetConstraints(Constraint)`, `SetReason(std::string)`
- `operator==`/`operator<=>` (defaulted)

### `dependency.cpp`/`dependency_impl.hpp` — `Dependency`
- `Dependency()`, copy/move ctor+assign, `~Dependency()`
- `GetName() const noexcept`, `GetConstraints() const noexcept`, `IsOptional() const noexcept`
- `SetName(std::string)`, `SetConstraints(Constraint)`, `SetOptional(bool)`
- `operator==`/`operator<=>` (defaulted)

### `manifest.cpp` — `Manifest`
- `Result<void> Resolve()` — collapses inline and by-name prototype/enum definitions into one shared table per manifest, then rewrites every reference to point at its definition; detects duplicate-name conflicts and reference cycles.
- `Result<void> Validate() const` — checks invariants the JSON schema can't express (name uniqueness across methods/prototypes/classes, `varIndex` in range, handleless classes without constructors, etc.) — assumes `Resolve()` already ran.
- `void ResolvePaths(const std::filesystem::path& base, const std::filesystem::path& file)` — fills in the runtime library filename/path and rebases declared directories.

*(Internal machinery: an anonymous-namespace `TypeTable` does the two-pass hoist-then-link resolution and a DFS-based cycle check; not itemized here — see `src-core.md`.)*

### `libsolv_dependency_resolver.hpp`/`.cpp` — `LibsolvDependencyResolver` (implements `IDependencyResolver`)
- `LibsolvDependencyResolver(std::shared_ptr<ILogger>)`
- `ResolutionReport Resolve(std::span<const Extension>) override` — the only public entry point; builds a libsolv `Pool`/`Repo` from the extensions' manifests, runs the SAT solver, and translates its output (install set, conflicts, ordering) into a `ResolutionReport`.

*(Private helpers — pool setup, solvable/dependency/conflict/obsolete registration, constraint conversion, solver-problem extraction, install-order computation — all wrap specific libsolv C API calls; see `src-jit-platform.md`-style prose coverage in `src-core.md` for how the pool/repo/solver/transaction pieces fit together, not reproduced here.)*

### `provider.cpp` — `Provider` (the read-only handle passed to language modules)
- `Provider(const ServiceLocator&, const Config&, const Manager&)`, copy/move ctor+assign, `~Provider()`
- `void Log(std::string_view, Severity, const Location&) const` — forwards to the registered `ILogger`.
- `bool IsPreferOwnSymbols() const noexcept`
- `GetBaseDir/GetExtensionsDir/GetConfigsDir/GetDataDir/GetLogsDir/GetCacheDir() const noexcept`
- `bool IsExtensionLoaded(std::string_view, std::optional<Constraint>) const noexcept`
- `const Extension* FindExtension(std::string_view) const noexcept` / `FindExtension(UniqueId) const noexcept`
- `std::vector<const Extension*> GetExtensions() const`
- `operator==`/`operator<=>` (defaulted)
- `const ServiceLocator& GetServices() const noexcept`

### `registrar.cpp` — `Registrar`, free function `plugify::ToString`
- `Registrar(UniqueId, std::string name)` — registers the id→name mapping in a process-wide, mutex-guarded map.
- `~Registrar()` — unregisters it.
- Move ctor/assign (no copy).
- `const std::string& plugify::ToString(UniqueId) noexcept` — looks up a registered id's name, `"<unknown>"` if not found.

### `service_locator.cpp` — `ServiceLocator`, `ServiceLocator::ServiceBuilder`, `ScopedServiceLocator`
**`ServiceLocator`** (a small dependency-injection container keyed by `std::type_index`)
- `ServiceLocator()`, move ctor/assign (no copy), `~ServiceLocator()`
- `RegisterInstanceInternal(std::type_index, std::shared_ptr<void>)` — registers a singleton instance.
- `RegisterFactoryInternal(std::type_index, std::function<std::shared_ptr<void>()>, ServiceLifetime)` — registers a factory (singleton/transient/scoped lifetime).
- `std::shared_ptr<void> ResolveInternal(std::type_index) const` — throws if unregistered.
- `std::shared_ptr<void> TryResolveInternal(std::type_index) const noexcept` — null if unregistered.
- `bool IsRegisteredInternal(std::type_index) const`
- `BeginScope()` / `EndScope()` — push/pop a thread-local scope for `Scoped`-lifetime services.
- `Clear()`, `size_t Count() const`
- `ServiceBuilder Services()` — entry point for the typed registration helper (templates on the public header wrap the `*Internal` calls above with `typeid(T)`).

**`ScopedServiceLocator`** — RAII: `BeginScope()` on construction, `EndScope()` on destruction.

### `glaze_metadata.hpp` — `glz::meta<T>` specializations
Declares `glz::meta<...>` for `Method`, `Manifest`, and (per the same pattern) the other manifest value types — each one maps a JSON field name to the corresponding `_impl` member (or, for `Manifest` itself, to public data members directly), so glaze can serialize/deserialize these PIMPL types without their fields being independently reflectable. Purely declarative field-name-to-member mappings, not logic.

### `glaze_adapter.hpp` — `GlazeAdapter`, `GlazeArray`, `GlazeObject`, `GlazeValue`, `GlazeFrozenValue`, and their iterators
A valijson adapter (per that library's `BasicAdapter` extension contract) letting valijson validate glaze's generic JSON DOM (`glz::generic`, or `glz::json_t` on older glaze releases) as an in-memory document, rather than requiring a separate parse into valijson's own document type. Class list per the file's own header comment: `GlazeAdapter`, `GlazeArray`, `GlazeArrayValueIterator`, `GlazeFrozenValue`, `GlazeObject`, `GlazeObjectMember`, `GlazeObjectMemberIterator`, `GlazeValue` — each implements one piece of valijson's adapter interface (array/object/value access) over glaze's generic-JSON container types. Signatures not itemized here since this is a fairly mechanical interface-conformance shim; see valijson's own `Adapter` concept for what each class is required to implement.

---

## Pipeline

### `pipeline.hpp` — `Pipeline<T>`, `Pipeline<T>::Builder`, `Pipeline<T>::Report`, `StageStatistics`
- `Pipeline<T>::Builder::Create()` (static) → `Builder`
- `Builder::AddStage<StageType>(std::unique_ptr<StageType>, bool required = true)` — fluent.
- `Builder::WithThreadPoolSize(size_t)` — fluent.
- `Builder::Build()` → `std::unique_ptr<Pipeline<T>>`
- `Pipeline<T>::Execute(std::vector<T>& items)` → `Report` — runs every added stage in order over `items` (modified in place), stopping early if a required stage fails.
- `Report::Summary() const` / `Report::Error() const` — human-readable reports.

*(Internally dispatches each stage by its `StageType` — `Transform` runs items in parallel via a `glz::pool` thread pool, `Barrier` processes the whole container at once (can reorder/filter), `Sequential` processes in order with an early-stop-on-error option — see `src-core.md`.)*

### `stages.hpp` — `IStage<T>`, `ITransformStage<T>`, `IBarrierStage<T>`, `ISequentialStage<T>`, `ExecutionContext<T>`, `StageType`
- `IStage<T>::GetName() const` (pure), `GetType() const` (pure), `Setup(...)`/`Teardown(...)` (optional hooks).
- `ITransformStage<T>::ProcessItem(T&, const ExecutionContext<T>&)` (pure), `ShouldProcess(const T&) const` (optional filter).
- `IBarrierStage<T>::ProcessAll(std::vector<T>&, const ExecutionContext<T>&)` (pure) — can reorder/filter the whole container.
- `ISequentialStage<T>::ProcessItem(T&, size_t position, size_t total, const ExecutionContext<T>&)` (pure), `ContinueOnError() const`, `ShouldProcess(const T&) const`.

### `stages_impl.hpp` — the four concrete pipeline stages plus one shared base
- `ParsingStage : ITransformStage<Extension>` — `ParsingStage(std::shared_ptr<IFileSystem>)`; parses+resolves+validates each extension's manifest against its JSON schema.
- `ResolutionStage : IBarrierStage<Extension>` — `ResolutionStage(resolver, loadOrder*, depGraph*, reverseDepGraph*, config)`; filters by whitelist/blacklist/platform, adds implicit language-module dependencies, runs the dependency resolver, reorders the extension list into load order.
- `BaseFailurePropagatingStage<Derived> : ISequentialStage<Extension>` — CRTP base shared by the three stages below; skips an item if any of its dependencies already failed, and propagates a new failure to that item's dependents.
- `LoadingStage : BaseFailurePropagatingStage<LoadingStage>` — loads modules, then plugins through their module's language binding.
- `ExportingStage : BaseFailurePropagatingStage<ExportingStage>` — exports each plugin's methods through every currently-running module.
- `StartingStage : BaseFailurePropagatingStage<StartingStage>` — calls each plugin's start hook, marks it running.

---

## Utilities

### `console_logger.hpp` — `ConsoleLogger` (implements `ILogger`)
- `ConsoleLogger(Severity minSeverity = Info)`
- `void Log(std::string_view, Severity, const Location&) override` — writes to stdout/stderr by severity, below-threshold messages dropped.
- `void SetLogLevel(Severity) override`, `Severity GetLogLevel() override`
- `void Flush() override`
- `protected: static std::string FormatMessage(std::string_view, Severity, const Location&)` — timestamped, leveled line format.

### `file_logger.hpp` — `FileLogger : ConsoleLogger`
- `FileLogger(std::filesystem::path logFile, Severity minSeverity = Info, size_t maxFileSize = 10MB)` — opens a timestamped log file.
- `~FileLogger()`
- `void Log(...) override` — writes to file, also echoes warnings/errors to console; rotates the file (closes, opens a fresh timestamped path) once it exceeds `maxFileSize`.
- `void Flush() override`

### `failure_tracker.hpp` — `FailureTracker`
- `FailureTracker(size_t capacity)`
- `void MarkFailed(UniqueId)`
- `bool HasFailed(UniqueId) const`
- `bool HasAnyDependencyFailed(const Extension&, const reverseDeps map&) const`
- `std::string GetFailedDependencyName(const Extension&, const reverseDeps map&) const`

### `assembly_handle.hpp` — `AssemblyHandle`
- `AssemblyHandle()`, `AssemblyHandle(void* handle, std::shared_ptr<IPlatformOps>, std::filesystem::path)`
- `~AssemblyHandle()` — unloads the library via the platform ops.
- Move-only (copy deleted).
- `GetHandle() const`, `GetPath() const`, `IsValid() const`
- `Result<Address> GetSymbol(std::string_view) const`

### `basic_assembly.hpp` — `BasicAssembly : IAssembly`
- `BasicAssembly(std::unique_ptr<AssemblyHandle>, std::shared_ptr<IPlatformOps>)`
- `GetSymbol(std::string_view) const override`, `IsValid() const override`, `GetPath() const override`, `GetBase() const override`, `GetHandle() const override`

### `basic_assembly_loader.hpp` — `BasicAssemblyLoader : IAssemblyLoader`
- `BasicAssemblyLoader(std::shared_ptr<IPlatformOps>, std::shared_ptr<IFileSystem>)`
- `Result<AssemblyPtr> Load(const std::filesystem::path&, LoadFlag, std::span<const std::filesystem::path> searchPaths) override` — resolves the path, checks a weak-pointer cache, temporarily adds runtime search paths if the platform supports it, loads via platform ops, caches the result.
- `Result<void> Unload(const AssemblyPtr&) override` — drops it from the cache.

### `standart_file_system.hpp`/`.cpp` — `StandardFileSystem : IFileSystem`, `ExtendedFileSystem : StandardFileSystem`
**`StandardFileSystem`** (thin wrapper over `std::filesystem` + fstream)
- `ReadTextFile/ReadBinaryFile(path) override`, `WriteTextFile/WriteBinaryFile(path, data) override`
- `IsExists/IsDirectory/IsRegularFile(path) override`
- `GetFileInfo(path) override`, `ListDirectory(dir) override`, `IterateDirectory(dir, options) override`
- `FindFiles(dir, patterns, recursive) override`
- `CreateDirectories/Remove/RemoveAll(path) override`, `Copy/Move(from, to) override`
- `GetAbsolutePath/GetCanonicalPath(path) override`, `GetRelativePath(path, base) override`
- `protected: static GetError()/SetError(int)/GetStringError(int)/GetStreamError(...)` — errno/streamstate-to-message helpers.

**`ExtendedFileSystem`** (adds, non-virtual)
- `AppendTextFile/AppendBinaryFile(path, data)`
- `WriteFileAtomic(path, content)`
- `GetSpaceInfo(path)`, `CreateTempFile(dir, prefix)`, `CreateTempDirectory(dir, prefix)`
- `FilesEqual(path1, path2)`, `ComputeSimpleHash(path)`
- `GetPermissions/SetPermissions(path, perms)`
- `IsWritable/IsReadable(path)`, `GetLastWriteTime/SetLastWriteTime(path, time)`
- `IsSymlink(path)`, `CreateSymlink(target, link)`, `ReadSymlink(path)`
