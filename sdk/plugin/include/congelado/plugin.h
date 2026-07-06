// NOLINTBEGIN
#pragma once
#include <congelado/mode.h>
#include <congelado/abi.h>
// Plugin SDK macro header.
// Use 'import congelado_plugin;' for C++ types (Plugin, free functions).
// Include this header for CONGELADO_PLUGIN(T) and CONGELADO_CAP_* macros.
// Requires: import congelado_plugin; before use (provides congelado::Plugin, free functions).
#include <string_view>

// ── Capability bitmask ───────────────────────────────────────────────────────
#define CONGELADO_CAP_LOGGER   1u
#define CONGELADO_CAP_PROTOCOL 2u
#define CONGELADO_CAP_STORAGE  4u
#define CONGELADO_CAP_CUSTOM   8u

// ── CONGELADO_PLUGIN(T) ───────────────────────────────────────────────────────
// Generates all C dlsym symbols from a congelado::Plugin subclass.
// The C ABI is internal — plugin authors write pure C++; the macro bridges to dlsym.
// Drop exactly once at the bottom of your plugin .cc, after the class definition.
#ifdef CONGELADO_GUEST

#if defined(CONGELADO_TASK_USED)
#error "CONGELADO_PLUGIN cannot be used in the same translation unit as CONGELADO_TASK; move plugin definitions to a separate file."
#endif

#ifndef CONGELADO_PLUGIN_USED
#define CONGELADO_PLUGIN_USED

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
    extern "C" int congelado_init(const CongeladoHostCallbacks *host,                                \
                                  const CongeladoConfigView *cfg) noexcept {                         \
        if (s_plugin == nullptr) s_plugin = new T{};                                                 \
        try {                                                                                        \
            s_plugin->on_load(host ? *host : CongeladoHostCallbacks{},                               \
                              cfg ? *cfg : CongeladoConfigView{});                                   \
        } catch (...) {                                                                              \
            return -1;                                                                               \
        }                                                                                            \
        return 0;                                                                                    \
    }                                                                                                \
    extern "C" const char *congelado_type() noexcept {                                               \
        if (s_plugin == nullptr) s_plugin = new T{};                                                 \
        return s_plugin->get_type().data();                                                          \
    }                                                                                                \
    extern "C" const char *congelado_worker_type() noexcept {                                        \
        if (s_plugin == nullptr) s_plugin = new T{};                                                 \
        return s_plugin->get_worker_type().data();                                                   \
    }                                                                                                \
    extern "C" CongeladoConfigView congelado_worker_execute(                                          \
                                      const CongeladoConfigView *input) noexcept {                    \
        if (s_plugin == nullptr) return {};                                                          \
        return s_plugin->execute_worker(input);                                                      \
    }                                                                                                     \
    /* Lifecycle: expose standardized shared names (no _plugin suffix) */                                 \
    extern "C" void congelado_on_unload() noexcept {                                                   \
        if (s_plugin != nullptr) {                                                                        \
            s_plugin->on_unload();                                                                        \
            delete s_plugin; /* NOLINT(cppcoreguidelines-owning-memory) */                               \
            s_plugin = nullptr;                                                                           \
        }                                                                                                 \
    }                                                                                                     \
    extern "C" void congelado_on_ready() noexcept {                                                    \
        if (s_plugin != nullptr)                                                                          \
            s_plugin->on_ready();                                                                         \
    }                                                                                                     \
    extern "C" int congelado_on_reload_requested() noexcept {                                            \
        return s_plugin != nullptr && s_plugin->on_reload_requested() ? 1 : 0;                           \
    }                                                                                                     \
    extern "C" void congelado_logger_write(int level, const char *msg, size_t len) noexcept {            \
        if (s_plugin != nullptr)                                                                          \
            ::congelado::_cap_dispatch::logger_write(s_plugin, level,                                    \
                                                      std::string_view{msg, len});                         \
    }                                                                                                     \
    extern "C" void congelado_logger_write_error(const char *msg, size_t len) noexcept {                 \
        if (s_plugin != nullptr)                                                                          \
            ::congelado::_cap_dispatch::logger_write(s_plugin, 4,                                        \
                                                      std::string_view{msg, len});                         \
    }                                                                                                     \
    extern "C" void *congelado_protocol_get() noexcept {                                                 \
        return s_plugin != nullptr ? ::congelado::_cap_dispatch::protocol_get(s_plugin) : nullptr;       \
    }                                                                                                     \
    extern "C" void *congelado_storage_get() noexcept {                                                  \
        return s_plugin != nullptr ? ::congelado::_cap_dispatch::storage_get(s_plugin) : nullptr;        \
    }                                                                                                     \
    extern "C" const char *congelado_unique_type() noexcept {                                            \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_unique_type().data();                                                        \
    }                                                                                                     \
    extern "C" const char *const *congelado_requires() noexcept {                                        \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        static std::vector<std::string> s_strs;        /* NOLINT */                                     \
        static std::vector<const char *> s_ptrs;       /* NOLINT */                                     \
        static bool s_cache_built = false;              /* NOLINT */                                     \
        if (!s_cache_built) {                                                                             \
            for (auto sv : s_plugin->get_requires()) {                                                   \
                s_strs.emplace_back(sv);                                                                 \
                s_ptrs.push_back(s_strs.back().c_str());                                                 \
            }                                                                                             \
            s_cache_built = true;                                                                         \
        }                                                                                                 \
        return s_ptrs.data();                                                                             \
    }                                                                                                     \
    extern "C" std::size_t congelado_requires_count() noexcept {                                         \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_requires().size();                                                           \
    }                                                                                                     \
    extern "C" const char *const *congelado_load_before_types() noexcept {                               \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        static std::vector<std::string> s_strs;        /* NOLINT */                                     \
        static std::vector<const char *> s_ptrs;       /* NOLINT */                                     \
        static bool s_cache_built = false;              /* NOLINT */                                     \
        if (!s_cache_built) {                                                                             \
            for (auto sv : s_plugin->get_load_before_types()) {                                         \
                s_strs.emplace_back(sv);                                                                 \
                s_ptrs.push_back(s_strs.back().c_str());                                                 \
            }                                                                                             \
            s_cache_built = true;                                                                         \
        }                                                                                                 \
        return s_ptrs.data();                                                                             \
    }                                                                                                     \
    extern "C" std::size_t congelado_load_before_types_count() noexcept {                                \
        if (s_plugin == nullptr) s_plugin = new T{};                                                     \
        return s_plugin->get_load_before_types().size();                                                  \
    }
#endif // CONGELADO_PLUGIN_USED

#endif // CONGELADO_GUEST

// NOLINTEND
