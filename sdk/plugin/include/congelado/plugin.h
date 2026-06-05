// NOLINTBEGIN
#pragma once
// Plugin SDK macro header.
// Use 'import congelado_plugin;' for C++ types (Plugin, HostCallbacks, ConfigView).
// Include this header for CONGELADO_PLUGIN(T) and CONGELADO_CAP_* macros.
// Requires: import congelado_plugin; before use (provides congelado::Plugin, HostCallbacks, ConfigView).
#include <stddef.h>   // size_t
#include <stdint.h>   // uint32_t
#include <string_view>

// ── C ABI — shared contract between bridge and plugins ────────────────────────
// These types are internal to the plugin loading mechanism.
// Plugin authors interact only with the C++ wrappers from import congelado_plugin.

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*congelado_log_fn)(void *ctx, int level, const char *msg, size_t len);
typedef void (*congelado_sched_fn)(void *ctx);

typedef struct CongeladoHostCallbacks {
    congelado_log_fn   log;
    congelado_sched_fn schedule;
    void              *router_ctx;
    void              *controller_ctx;
    void              *leverager_ctx;
    void              *ctx;
} CongeladoHostCallbacks;

typedef struct CongeladoConfigView {
    const char *const *keys;
    const char *const *values;
    size_t             count;
} CongeladoConfigView;

#ifdef __cplusplus
}
#endif

// ── Capability bitmask ────────────────────────────────────────────────────────
#define CONGELADO_CAP_LOGGER   1u
#define CONGELADO_CAP_PROTOCOL 2u
#define CONGELADO_CAP_STORAGE  4u
#define CONGELADO_CAP_CUSTOM   8u

// ── CONGELADO_PLUGIN(T) ───────────────────────────────────────────────────────
// Generates all C dlsym symbols from a congelado::Plugin subclass.
// The C ABI is internal — plugin authors write pure C++; the macro bridges to dlsym.
// Drop exactly once at the bottom of your plugin .cc, after the class definition.
#define CONGELADO_PLUGIN(T) /* NOLINT(cppcoreguidelines-macro-usage) */                                  \
    static T *s_plugin = nullptr; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */       \
    extern "C" const char *congelado_plugin_name() noexcept {                                            \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_name().data();                                                               \
    }                                                                                                     \
    extern "C" const char *congelado_plugin_version() noexcept {                                         \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_version().data();                                                                \
    }                                                                                                     \
    extern "C" uint32_t congelado_capabilities() noexcept {                                              \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->capabilities();                                                                  \
    }                                                                                                     \
    extern "C" void congelado_on_load(const CongeladoHostCallbacks *host,                                \
                                      const CongeladoConfigView    *cfg) {                               \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        s_plugin->on_load(                                                                                \
            ::congelado::HostCallbacks{host->log, host->schedule, host->router_ctx,                      \
                                       host->controller_ctx, host->leverager_ctx, host->ctx},            \
            ::congelado::ConfigView{                                                                      \
                cfg != nullptr ? cfg->keys   : nullptr,                                                  \
                cfg != nullptr ? cfg->values : nullptr,                                                  \
                cfg != nullptr ? cfg->count  : 0                                                         \
            });                                                                                           \
    }                                                                                                     \
    extern "C" void congelado_on_unload() noexcept {                                                     \
        if (s_plugin != nullptr) {                                                                        \
            s_plugin->on_unload();                                                                        \
            delete s_plugin; /* NOLINT(cppcoreguidelines-owning-memory) */                               \
            s_plugin = nullptr;                                                                           \
        }                                                                                                 \
    }                                                                                                     \
    extern "C" void congelado_logger_write(int level, const char *msg, size_t len) noexcept {            \
        if (s_plugin != nullptr)                                                                          \
            s_plugin->logger_write(level, std::string_view{msg, len});                                   \
    }                                                                                                     \
    extern "C" void congelado_logger_write_error(const char *msg, size_t len) noexcept {                 \
        if (s_plugin != nullptr)                                                                          \
            s_plugin->logger_write(4, std::string_view{msg, len});                                       \
    }                                                                                                     \
    extern "C" void *congelado_protocol_get() noexcept {                                                 \
        return s_plugin != nullptr ? s_plugin->protocol_get() : nullptr;                                 \
    }                                                                                                     \
    extern "C" void *congelado_storage_get() noexcept {                                                  \
        return s_plugin != nullptr ? s_plugin->storage_get() : nullptr;                                  \
    }                                                                                                     \
    extern "C" const char *congelado_unique_type() noexcept {                                            \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_unique_type().data();                                                        \
    }                                                                                                     \
    extern "C" const char *const *congelado_requires() noexcept {                                        \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        static std::vector<const char *> s_cache;       /* NOLINT */                                     \
        static bool s_cache_built = false;              /* NOLINT */                                     \
        /* Pointers into static/constexpr literals — valid for program lifetime. Built once. */          \
        if (!s_cache_built) {                                                                             \
            for (auto sv : s_plugin->get_requires())                                                     \
                s_cache.push_back(sv.data());                                                             \
            s_cache_built = true;                                                                         \
        }                                                                                                  \
        return s_cache.data();                                                                            \
    }                                                                                                     \
    extern "C" std::size_t congelado_requires_count() noexcept {                                         \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_requires().size();                                                           \
    }                                                                                                     \
    extern "C" const char *const *congelado_load_before_types() noexcept {                               \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        static std::vector<const char *> s_cache;       /* NOLINT */                                     \
        static bool s_cache_built = false;              /* NOLINT */                                     \
        if (!s_cache_built) {                                                                             \
            for (auto sv : s_plugin->get_load_before_types())                                            \
                s_cache.push_back(sv.data());                                                             \
            s_cache_built = true;                                                                         \
        }                                                                                                 \
        return s_cache.data();                                                                            \
    }                                                                                                     \
    extern "C" std::size_t congelado_load_before_types_count() noexcept {                                \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_load_before_types().size();                                                  \
    }
// NOLINTEND
