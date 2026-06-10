module sdk.plugin.plugin;

// PORT-NOTE: SDK C header — extern(C) struct ABI types only.
// No Object base, no exceptions, no GC.

import core.stdc.stddef : size_t;
import core.stdc.stdint : uint32_t;

// ── C ABI — shared contract between bridge and plugins ────────────────────────
// These types are internal to the plugin loading mechanism.
// Plugin authors interact only with the C++ wrappers from congelado_plugin.d.

extern(C):

alias congelado_log_fn   = void function(void* ctx, int level, const(char)* msg, size_t len) nothrow @nogc;
alias congelado_sched_fn = void function(void* ctx) nothrow @nogc;

// PORT-NOTE: ABI vtable struct — maps exactly to the C layout.
extern(C) struct CongeladoHostCallbacks {
    congelado_log_fn   log;
    congelado_sched_fn schedule;
    void*              router_ctx;
    void*              controller_ctx;
    void*              leverager_ctx;
    void*              ctx;
}

extern(C) struct CongeladoConfigView {
    const(char*)* keys;
    const(char*)* values;
    size_t        count;
}

// ── Capability bitmask ────────────────────────────────────────────────────────
enum uint CONGELADO_CAP_LOGGER   = 1u;
enum uint CONGELADO_CAP_PROTOCOL = 2u;
enum uint CONGELADO_CAP_STORAGE  = 4u;
enum uint CONGELADO_CAP_CUSTOM   = 8u;

// ── CongeladoPlugin mixin template ───────────────────────────────────────────
// Equivalent of CONGELADO_PLUGIN(T) macro — generates all extern(C) dlsym symbols
// from a congelado.Plugin subclass.
// Drop exactly once at the bottom of your plugin .d, after the class definition.
//
// PORT-NOTE: C++ macro used a static T* s_plugin with lazy init.
// D mixin uses a __gshared pointer with the same lazy-init pattern.
// The macro body uses new T{} for construction; D uses scope auto / make!T.
// No @nogc on these exported symbols (plugin binary boundary; GC allowed in plugin).
mixin template CongeladoPlugin(T) {
    import sdk.plugin.plugin : CongeladoHostCallbacks, CongeladoConfigView;
    import sdk.plugin.congelado_plugin : HostCallbacks, ConfigView;

    __gshared T* s_plugin = null;

    extern(C) export const(char)* congelado_plugin_name() nothrow {
        if (s_plugin is null) s_plugin = new T();
        return s_plugin.get_name().ptr;
    }
    extern(C) export const(char)* congelado_plugin_version() nothrow {
        if (s_plugin is null) s_plugin = new T();
        return s_plugin.get_version().ptr;
    }
    extern(C) export uint32_t congelado_capabilities() nothrow {
        if (s_plugin is null) s_plugin = new T();
        return s_plugin.capabilities();
    }
    extern(C) export void congelado_on_load(const(CongeladoHostCallbacks)* host,
                                             const(CongeladoConfigView)* cfg) {
        if (s_plugin is null) s_plugin = new T();
        auto hcb = HostCallbacks(
            host.log, host.schedule,
            host.router_ctx, host.controller_ctx, host.leverager_ctx, host.ctx);
        ConfigView cv;
        if (cfg !is null)
            cv = ConfigView(cfg.keys, cfg.values, cfg.count);
        s_plugin.on_load(hcb, cv);
    }
    extern(C) export void congelado_on_unload() nothrow {
        if (s_plugin !is null) {
            s_plugin.on_unload();
            // PORT-NOTE: C++ deleted s_plugin; D uses destroy + free for @nogc compatibility.
            // Simple null-out here; Run 2 should use util.alloc.dispose.
            s_plugin = null;
        }
    }
    extern(C) export void congelado_logger_write(int level, const(char)* msg, size_t len) nothrow {
        if (s_plugin !is null)
            s_plugin.logger_write(level, msg[0 .. len]);
    }
    extern(C) export void congelado_logger_write_error(const(char)* msg, size_t len) nothrow {
        if (s_plugin !is null)
            s_plugin.logger_write(4, msg[0 .. len]);
    }
    extern(C) export void* congelado_protocol_get() nothrow {
        return s_plugin !is null ? s_plugin.protocol_get() : null;
    }
    extern(C) export void* congelado_storage_get() nothrow {
        return s_plugin !is null ? s_plugin.storage_get() : null;
    }
    extern(C) export const(char)* congelado_unique_type() nothrow {
        if (s_plugin is null) s_plugin = new T();
        return s_plugin.get_unique_type().ptr;
    }
    // PORT-NOTE: C++ used a static std::vector<const char*> cache.
    // D uses a __gshared fixed buffer.
    extern(C) export const(char*)* congelado_requires() nothrow {
        if (s_plugin is null) s_plugin = new T();
        auto reqs = s_plugin.get_requires();
        // Return pointer to internal static array of .ptr values.
        __gshared const(char)*[64] s_cache;
        __gshared bool             s_built = false;
        if (!s_built) {
            size_t i = 0;
            foreach (sv; reqs) {
                if (i >= 64) break;
                s_cache[i++] = sv.ptr;
            }
            s_built = true;
        }
        return s_cache.ptr;
    }
    extern(C) export size_t congelado_requires_count() nothrow {
        if (s_plugin is null) s_plugin = new T();
        return s_plugin.get_requires().length;
    }
    extern(C) export const(char*)* congelado_load_before_types() nothrow {
        if (s_plugin is null) s_plugin = new T();
        auto lbt = s_plugin.get_load_before_types();
        __gshared const(char)*[64] s_cache;
        __gshared bool             s_built = false;
        if (!s_built) {
            size_t i = 0;
            foreach (sv; lbt) {
                if (i >= 64) break;
                s_cache[i++] = sv.ptr;
            }
            s_built = true;
        }
        return s_cache.ptr;
    }
    extern(C) export size_t congelado_load_before_types_count() nothrow {
        if (s_plugin is null) s_plugin = new T();
        return s_plugin.get_load_before_types().length;
    }
}
