module sdk.plugin.congelado_plugin;

// PORT-NOTE: C++ used std::span<const std::string_view> for get_requires /
// get_load_before_types. D uses const(char)[][] (caller-owned slice of string views).
// Plugin pointers are raw (no shared_ptr): plugin lifetime is managed by the host.

import core.stdc.stddef : size_t;
import core.stdc.stdint : uint32_t;

import sdk.plugin.plugin : CongeladoHostCallbacks, CongeladoConfigView,
                           CONGELADO_CAP_LOGGER, CONGELADO_CAP_PROTOCOL,
                           CONGELADO_CAP_STORAGE, CONGELADO_CAP_CUSTOM;

// ─── HostCallbacks ────────────────────────────────────────────────────────────

// C++ view of the host callback table. Constructed by CongeladoPlugin from C ABI args.
// Valid only for the duration of on_load(). Do not store.
class HostCallbacks {
  public:
    alias LogFn   = void function(void* ctx, int level, const(char)* msg, size_t len) nothrow @nogc;
    alias SchedFn = void function(void* ctx) nothrow @nogc;

    this(LogFn log_, SchedFn sched_, void* router_ctx_,
         void* controller_ctx_, void* leverager_ctx_, void* ctx_) nothrow @nogc {
        m_log            = log_;
        m_sched          = sched_;
        m_router_ctx     = router_ctx_;
        m_controller_ctx = controller_ctx_;
        m_leverager_ctx  = leverager_ctx_;
        m_ctx            = ctx_;
    }

    void log(int level, const(char)[] msg) const nothrow @nogc {
        if (m_log !is null)
            m_log(m_ctx, level, msg.ptr, msg.length);
    }
    void schedule() const nothrow @nogc {
        if (m_sched !is null)
            m_sched(m_ctx);
    }
    // Typed accessors — C ABI carries void* across the dlopen boundary; cast happens here.
    T* router_ctx(T)()     const nothrow @nogc { return cast(T*) m_router_ctx;     }
    T* controller_ctx(T)() const nothrow @nogc { return cast(T*) m_controller_ctx; }
    T* leverager_ctx(T)()  const nothrow @nogc { return cast(T*) m_leverager_ctx;  }

  private:
    LogFn   m_log            = null;
    SchedFn m_sched          = null;
    void*   m_router_ctx     = null;
    void*   m_controller_ctx = null;
    void*   m_leverager_ctx  = null;
    void*   m_ctx            = null;
}

// ─── ConfigView ───────────────────────────────────────────────────────────────

// Read-only view of the plugin's config section.
// Valid only for the duration of on_load(). Do not store.
class ConfigView {
  public:
    this() nothrow @nogc {}
    this(const(char*)* keys_, const(char*)* values_, size_t count_) nothrow @nogc {
        m_keys   = keys_;
        m_values = values_;
        m_count  = count_;
    }

    size_t size()  const nothrow @nogc { return m_count;       }
    bool   empty() const nothrow @nogc { return m_count == 0;  }

    // Returns pointer to value string for key, or null if not found.
    // PORT-NOTE: C++ returned std::optional<std::string_view>; D returns null or ptr.
    const(char)[] get(const(char)[] key) const nothrow @nogc {
        for (size_t i = 0; i < m_count; ++i) {
            import core.stdc.string : strlen, strncmp;
            const(char)* k = m_keys[i];
            size_t klen = strlen(k);
            if (klen == key.length && strncmp(k, key.ptr, klen) == 0)
                return m_values[i][0 .. strlen(m_values[i])];
        }
        return null;
    }

    // for_each(callback): invoke callback(key, value) for each config entry.
    void for_each(void delegate(const(char)[], const(char)[]) @nogc nothrow cb)
            const nothrow @nogc {
        import core.stdc.string : strlen;
        for (size_t i = 0; i < m_count; ++i) {
            const(char)[] k = m_keys[i][0 .. strlen(m_keys[i])];
            const(char)[] v = m_values[i][0 .. strlen(m_values[i])];
            cb(k, v);
        }
    }

  private:
    const(char*)* m_keys   = null;
    const(char*)* m_values = null;
    size_t        m_count  = 0;
}

// ─── Plugin ───────────────────────────────────────────────────────────────────

// C++ base class for plugin authors.
// Inherit this, override virtual methods, drop mixin CongeladoPlugin!T at the
// bottom of your .d file.
abstract class Plugin {
  public:
    // IMPORTANT: returned const(char)[] .ptr is used as a raw const char*.
    // Implementations MUST return a slice of a string literal or stable member.
    abstract const(char)[] get_name()    const nothrow;
    abstract const(char)[] get_version() const nothrow;

    uint32_t capabilities() const nothrow { return 0; }

    // Returns a type tag for uniqueness enforcement. Empty slice = not unique.
    // Only one plugin with a given tag loads; the first in config order wins.
    // MUST return a slice of a string literal.
    const(char)[] get_unique_type() const nothrow { return ""; }

    // Returns the names of plugins that must be loaded before this plugin.
    // Each name must exactly match get_name() of the required plugin.
    // MUST return a slice over a static array of string literals.
    const(const(char)[])[  ] get_requires()           const nothrow { return null; }

    // Returns unique-type tags this plugin must be loaded before.
    // MUST return a slice over a static array of string literals.
    const(const(char)[])[  ] get_load_before_types()  const nothrow { return null; }

    void on_load(HostCallbacks host, ConfigView cfg) {}
    void on_unload() {}
    void logger_write(int /*level*/, const(char)[] /*msg*/) nothrow {}
    void* protocol_get() nothrow { return null; }
    // Returns interfaces.IDatabase* cast to void*. Override with CONGELADO_CAP_STORAGE.
    void* storage_get() nothrow { return null; }
}
