module core.ffi.bridge;
@nogc nothrow:

import interfaces.logger  : ILogger, LogLevel;
import shared.handler     : HandlerBase, WorkerFunction, ReleaseFunction, ErrorHandler;
import core.config.types  : PluginConfig;
import util.alloc         : make, dispose;

// ── C ABI structs (mirrors congelado/plugin.h) ─────────────────────────────────

extern(C) {
    alias congelado_log_fn   = void function(void* ctx, int level, const(char)* msg, size_t len) @nogc nothrow;
    alias congelado_sched_fn = void function(void* ctx) @nogc nothrow;

    struct CongeladoHostCallbacks {
        congelado_log_fn   log;
        congelado_sched_fn schedule;
        void*              router_ctx;
        void*              controller_ctx;
        void*              leverager_ctx;
        void*              ctx;
    }

    struct CongeladoConfigView {
        const(char*)* keys;
        const(char*)* values;
        size_t        count;
    }
}

// ── Capability bitmask — mirrors CONGELADO_CAP_* defines in plugin.h ──────────
enum uint CONGELADO_CAP_LOGGER   = 1u;
enum uint CONGELADO_CAP_PROTOCOL = 2u;
enum uint CONGELADO_CAP_STORAGE  = 4u;
enum uint CONGELADO_CAP_CUSTOM   = 8u;

// Capability bitmask enum
enum Cap : uint {
    LOGGER   = CONGELADO_CAP_LOGGER,
    PROTOCOL = CONGELADO_CAP_PROTOCOL,
    STORAGE  = CONGELADO_CAP_STORAGE,
    CUSTOM   = CONGELADO_CAP_CUSTOM,
}

class LoadError {
  public:
    this(const(char)[] detail) { m_detail = detail; }

    const(char)[] get_detail() const { return m_detail; }

  private:
    const(char)[] m_detail;
}

// PORT-NOTE: libffi Closure → replaced by a simple struct holding the function pointer
//   and user_data, because D's @nogc cannot safely wrap libffi at the same level.
//   The log/schedule callbacks are assembled as plain C function pointers with a
//   void* context (the FfiBridge pointer), matching the C ABI without ffi_closure.
// PORT-NOTE: FfiBridge inherits HandlerBase, ILogger — both are D classes.
//   std::enable_shared_from_this → not needed; D uses plain reference counting or GC.
// PORT-NOTE: dlopen/dlclose/dlsym on Linux; LoadLibraryA/FreeLibrary/GetProcAddress on Win32.

// ── Platform DL helpers ────────────────────────────────────────────────────────
version(Windows) {
    private extern(Windows) {
        void*  LoadLibraryA(const(char)* path) @nogc nothrow;
        int    FreeLibrary(void* hmod) @nogc nothrow;
        void*  GetProcAddress(void* hmod, const(char)* name) @nogc nothrow;
    }
} else {
    private extern(C) {
        enum int RTLD_NOW   = 2;
        enum int RTLD_LOCAL = 0;
        void* dlopen(const(char)* path, int flags) @nogc nothrow;
        int   dlclose(void* handle) @nogc nothrow;
        void* dlsym(void* handle, const(char)* symbol) @nogc nothrow;
    }
}

// Resolved function pointers for a loaded plugin.
// All symbols are optional except name, version, capabilities.
private struct PluginSymbols {
    alias NameFn              = const(char)* function() @nogc nothrow;
    alias VersionFn           = const(char)* function() @nogc nothrow;
    alias CapsFn              = uint function() @nogc nothrow;
    alias OnLoadFn            = void function(const(CongeladoHostCallbacks)*, const(CongeladoConfigView)*) nothrow;
    alias OnUnloadFn          = void function() @nogc nothrow;
    alias LogWriteFn          = void function(int, const(char)*, size_t) @nogc nothrow;
    alias LogWriteErrFn       = void function(const(char)*, size_t) @nogc nothrow;
    alias ProtoGetFn          = void* function() @nogc nothrow;
    alias StorageGetFn        = void* function() @nogc nothrow;
    alias UniqueTypeFn        = const(char)* function() @nogc nothrow;
    alias RequiresFn          = const(char*)* function() @nogc nothrow;
    alias RequiresCountFn     = size_t function() @nogc nothrow;
    alias LoadBeforeTypesFn       = const(char*)* function() @nogc nothrow;
    alias LoadBeforeTypesCountFn  = size_t function() @nogc nothrow;

    NameFn              name              = null;
    VersionFn           version_          = null;
    CapsFn              capabilities      = null;
    OnLoadFn            on_load           = null;
    OnUnloadFn          on_unload         = null;
    LogWriteFn          logger_write      = null;
    LogWriteErrFn       logger_write_error = null;
    ProtoGetFn          protocol_get      = null;
    StorageGetFn        storage_get       = null;
    UniqueTypeFn        unique_type       = null;
    RequiresFn          requires_get      = null;
    RequiresCountFn     requires_count    = null;
    LoadBeforeTypesFn       load_before_types_get   = null;
    LoadBeforeTypesCountFn  load_before_types_count = null;
}

// PORT-NOTE: C++ log_thunk / schedule_thunk are extern(C) module-level functions
//   that receive a void* (FfiBridge pointer).  They cannot be lambdas under @nogc.
private extern(C) void log_thunk(void* ctx, int level, const(char)* msg, size_t len) @nogc nothrow {
    import core.stdc.stdio : fprintf, stderr;
    auto self = cast(FfiBridge)ctx;
    fprintf(stderr, "[plugin::%.*s] log(%d): %.*s\n",
            cast(int)self.get_name().length, self.get_name().ptr,
            level, cast(int)len, msg);
}

private extern(C) void schedule_thunk(void* ctx) @nogc nothrow {
    import core.stdc.stdio : fprintf, stderr;
    auto self = cast(FfiBridge)ctx;
    fprintf(stderr, "[plugin::%.*s] schedule requested\n",
            cast(int)self.get_name().length, self.get_name().ptr);
}

// RAII wrapper around one loaded plugin .so.
//
// Two-phase load:
//   open()     — dlopen + resolve symbols + read metadata (name, unique_type, requires)
//   activate() — build callbacks, call congelado_on_load, discover caps
class FfiBridge : HandlerBase, ILogger {
  public:
    @disable this(this);

    // Phase 1 of the two-phase load. Opens the .so, resolves symbols, reads metadata.
    // Does NOT call congelado_on_load. Call activate() after sorting.
    // PORT-NOTE: std::expected<shared_ptr<FfiBridge>, LoadError> → Result!(FfiBridge, LoadError)
    static FfiBridge open_bridge(const(char)[] path, const(PluginConfig) plugin_cfg = null,
                                  out LoadError err) {
        void* lib = open_lib(path);
        if (lib is null) {
            import util.alloc : make;
            err = make!LoadError("dlopen failed");
            return null;
        }

        auto bridge = make!FfiBridge(lib);

        const(char)[] sym_err = bridge.resolve_symbols();
        if (sym_err.length > 0) {
            err = make!LoadError(sym_err);
            dispose(bridge);
            return null;
        }

        const(char)* name_cstr = bridge.m_syms.name();
        import core.stdc.string : strlen;
        bridge.m_lib_name = name_cstr[0 .. strlen(name_cstr)];
        bridge.build_config_view(plugin_cfg);
        bridge.read_metadata();
        return bridge;
    }

    // Phase 2. Builds callbacks, calls congelado_on_load, discovers caps.
    // Must be called exactly once per bridge, after open().
    void activate(void* router_ctx = null, void* controller_ctx = null,
                  void* leverager_ctx = null) {
        CongeladoHostCallbacks callbacks = {
            log:              &log_thunk,
            schedule:         &schedule_thunk,
            router_ctx:       router_ctx,
            controller_ctx:   controller_ctx,
            leverager_ctx:    leverager_ctx,
            ctx:              cast(void*)this,
        };

        if (m_syms.on_load !is null)
            m_syms.on_load(&callbacks, &m_cfg_view);

        discover_caps();
    }

    ~this() {
        release_plugin();
        close_lib();
        // Free dynamic arrays
        m_requires.length      = 0;
        m_load_before_types.length = 0;
        m_cfg_keys.length      = 0;
        m_cfg_vals.length      = 0;
    }

    bool has(Cap cap) const {
        return (m_caps & cast(uint)cap) != 0;
    }

    override const(char)[] get_name() const { return m_lib_name; }

    const(char)[] get_unique_type() const { return m_unique_type; }
    const(char)[][] get_requires()         const { return cast(const(char)[][])m_requires; }
    const(char)[][] get_load_before_types() const { return cast(const(char)[][])m_load_before_types; }

    override void write(LogLevel level, const(char)[] msg) {
        if (m_syms.logger_write is null) return;
        m_syms.logger_write(cast(int)level, msg.ptr, msg.length);
    }

    override void error(const(char)[] msg) {
        if (m_syms.logger_write_error is null) return;
        m_syms.logger_write_error(msg.ptr, msg.length);
    }

    override WorkerFunction on_execute() { return null; }

    override ReleaseFunction on_released() {
        // PORT-NOTE: std::weak_ptr → capture self pointer; caller must ensure lifetime
        // Returns a closure that calls release_plugin if bridge is still alive.
        // Under @nogc we return a no-op — wiring weak-ref semantics deferred to Run 3.
        return null;
    }

    override ErrorHandler on_error() { return null; }

  private:
    this(void* lib) { m_lib = lib; }

    // ── Platform helpers ──────────────────────────────────────────────────────

    static void* open_lib(const(char)[] path) {
        // Need null-terminated path
        import core.stdc.stdlib : malloc, free;
        char* cpath = cast(char*)malloc(path.length + 1);
        if (cpath is null) return null;
        cpath[0 .. path.length] = path[];
        cpath[path.length] = '\0';
        void* handle;
        version(Windows) {
            handle = LoadLibraryA(cpath);
        } else {
            handle = dlopen(cpath, RTLD_NOW | RTLD_LOCAL);
        }
        free(cpath);
        return handle;
    }

    void close_lib() {
        if (m_lib is null) return;
        version(Windows) {
            FreeLibrary(m_lib);
        } else {
            dlclose(m_lib);
        }
        m_lib = null;
    }

    Fn probe(Fn)(const(char)* sym) const {
        version(Windows) {
            return cast(Fn)GetProcAddress(m_lib, sym);
        } else {
            return cast(Fn)dlsym(m_lib, sym);
        }
    }

    // ── Plugin lifecycle ──────────────────────────────────────────────────────

    // Returns an error string on failure, empty string on success.
    const(char)[] resolve_symbols() {
        m_syms.name         = probe!(PluginSymbols.NameFn)("congelado_plugin_name");
        m_syms.version_     = probe!(PluginSymbols.VersionFn)("congelado_plugin_version");
        m_syms.capabilities = probe!(PluginSymbols.CapsFn)("congelado_capabilities");

        if (m_syms.name is null || m_syms.version_ is null || m_syms.capabilities is null)
            return "missing required symbols: congelado_plugin_name / "
                   "congelado_plugin_version / congelado_capabilities";

        m_syms.on_load               = probe!(PluginSymbols.OnLoadFn)("congelado_on_load");
        m_syms.on_unload             = probe!(PluginSymbols.OnUnloadFn)("congelado_on_unload");
        m_syms.logger_write          = probe!(PluginSymbols.LogWriteFn)("congelado_logger_write");
        m_syms.logger_write_error    = probe!(PluginSymbols.LogWriteErrFn)("congelado_logger_write_error");
        m_syms.protocol_get          = probe!(PluginSymbols.ProtoGetFn)("congelado_protocol_get");
        m_syms.storage_get           = probe!(PluginSymbols.StorageGetFn)("congelado_storage_get");
        m_syms.unique_type           = probe!(PluginSymbols.UniqueTypeFn)("congelado_unique_type");
        m_syms.requires_get          = probe!(PluginSymbols.RequiresFn)("congelado_requires");
        m_syms.requires_count        = probe!(PluginSymbols.RequiresCountFn)("congelado_requires_count");
        m_syms.load_before_types_get   = probe!(PluginSymbols.LoadBeforeTypesFn)("congelado_load_before_types");
        m_syms.load_before_types_count = probe!(PluginSymbols.LoadBeforeTypesCountFn)("congelado_load_before_types_count");
        return "";
    }

    void build_config_view(const(PluginConfig) plugin_cfg) {
        if (plugin_cfg is null) return;
        // PORT-NOTE: iterating SwissHashMap fields to build key/value pointer arrays
        // Deferred: requires SwissHashMap iteration API in Run 2
        m_cfg_view = CongeladoConfigView(null, null, 0);
    }

    void discover_caps() {
        m_caps = m_syms.capabilities();

        // if (((m_caps & CONGELADO_CAP_PROTOCOL) != 0) && (m_syms.protocol_get != null)) { ... }
        // if (((m_caps & CONGELADO_CAP_STORAGE)  != 0) && (m_syms.storage_get  != null)) { ... }
    }

    void release_plugin() {
        if (m_syms.on_unload !is null) {
            m_syms.on_unload();
            m_syms = PluginSymbols.init;
        }
    }

    void read_metadata() {
        import core.stdc.string : strlen;
        if (m_syms.unique_type !is null) {
            const(char)* utype = m_syms.unique_type();
            m_unique_type = (utype !is null) ? utype[0 .. strlen(utype)] : "";
        }

        if (m_syms.requires_count !is null && m_syms.requires_get !is null) {
            const size_t count = m_syms.requires_count();
            const(char*)* arr = m_syms.requires_get();
            if (arr !is null) {
                m_requires.length = count;
                for (size_t i = 0; i < count; ++i)
                    if (arr[i] !is null)
                        m_requires[i] = arr[i][0 .. strlen(arr[i])];
            }
        }

        if (m_syms.load_before_types_count !is null && m_syms.load_before_types_get !is null) {
            const size_t count = m_syms.load_before_types_count();
            const(char*)* arr = m_syms.load_before_types_get();
            if (arr !is null) {
                m_load_before_types.length = count;
                for (size_t i = 0; i < count; ++i)
                    if (arr[i] !is null)
                        m_load_before_types[i] = arr[i][0 .. strlen(arr[i])];
            }
        }
    }

    // ── Members ───────────────────────────────────────────────────────────────

    void*          m_lib        = null;
    PluginSymbols  m_syms;
    const(char)[]  m_lib_name;
    const(char)[]  m_unique_type;
    const(char)[][] m_requires;
    const(char)[][] m_load_before_types;
    uint           m_caps = 0;
    // shared_ptr<interfaces::IProtocol> m_protocol;
    // shared_ptr<interfaces::IDatabase> m_storage;

    const(char*)[] m_cfg_keys;
    const(char*)[] m_cfg_vals;
    CongeladoConfigView m_cfg_view;
}
