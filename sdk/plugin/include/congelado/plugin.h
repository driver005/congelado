// NOLINTBEGIN
#pragma once
#include <congelado/abi.h>
#include <congelado/mode.h>
// Plugin SDK macro header.
// Use 'import congelado_plugin;' for C++ types (Plugin, free functions).
// Include this header for CONGELADO_PLUGIN(T) and CONGELADO_CAP_* macros.
// Requires: import congelado_plugin; before use (provides congelado::Plugin, free functions).
#include <deque>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

// ── Capability bitmask ───────────────────────────────────────────────────────
#define CONGELADO_CAP_LOGGER                1u
#define CONGELADO_CAP_PROTOCOL              2u
#define CONGELADO_CAP_STORAGE               4u
#define CONGELADO_CAP_CUSTOM                8u
#define CONGELADO_CAP_SERDE                 16u
#define CONGELADO_CAP_BRIDGE                32u
#define CONGELADO_CAP_OTEL                  64u
#define CONGELADO_CAP_OPENAPI               128u
#define CONGELADO_CAP_SEARCH                256u
#define CONGELADO_CAP_EVENTS                512u
#define CONGELADO_CAP_CACHE                 1'024u
#define CONGELADO_CAP_CRON                  2'048u
#define CONGELADO_CAP_WORKER_MANAGER        4'096u
#define CONGELADO_CAP_WORKER                8'192u
#define CONGELADO_CAP_APP_DEFS              16'384u
#define CONGELADO_CAP_WORKER_ORCHESTRATOR   32'768u
#define CONGELADO_CAP_WORKFLOW_ORCHESTRATOR 65'536u
#define CONGELADO_CAP_PAYLOAD_STORAGE       131'072u

// ── CONGELADO_PLUGIN(T) ───────────────────────────────────────────────────────
// Generates all C dlsym symbols from a congelado::Plugin subclass.
// The C ABI is internal — plugin authors write pure C++; the macro bridges to dlsym.
// Drop exactly once at the bottom of your plugin .cc, after the class definition.
//
// Exception contract: every generated symbol is noexcept, and every path that can
// allocate or call into plugin virtuals is guarded — a failed lazy construction
// (OOM or a throwing ctor) or a throwing execute_worker/on_* degrades to a null/empty
// result instead of std::terminate-ing the host process. The requires()/
// load_before_types() caches use std::deque so the cached c_str() pointers stay valid
// across growth (std::vector<std::string> reallocation + SSO would dangle them).
#ifdef CONGELADO_GUEST

#    if defined(CONGELADO_TASK_USED)
#        error                                                                                     \
            "CONGELADO_PLUGIN cannot be used in the same translation unit as CONGELADO_TASK; move plugin definitions to a separate file."
#    endif

#    ifndef CONGELADO_PLUGIN_USED
#        define CONGELADO_PLUGIN_USED

/**
 * @def CONGELADO_PLUGIN(T)
 * @brief Drops a lazily-constructed `static T *s_plugin` and every `extern "C"` symbol the host
 * dlsym's off a plugin `.so`, all wired straight to that one instance — this is the whole bridge
 * from pure-C++ `congelado::Plugin` subclasses to the C ABI, no cap.
 * @warning Exactly one invocation per translation unit (enforced by the
 * `CONGELADO_PLUGIN_USED` guard above), and it cannot coexist with `CONGELADO_TASK` in the same
 * TU — drop it once, at the bottom of your plugin `.cc`, after `T`'s definition. Mess up either
 * rule and it's a straight compile-time L via the `#error`s guarding this block.
 * @details Generated C symbols, each lazily constructing `s_plugin` on first touch if it isn't
 * already up (via the guarded `plugin_instance()` helper — thread-safe, allocation-failure-safe):
 * - `congelado_plugin_name()` → `T::get_name()`
 * - `congelado_plugin_version()` → `T::get_version()`
 * - `congelado_capabilities()` → `T::capabilities()`
 * - `congelado_init(host, cfg)` → constructs `s_plugin`, calls `T::on_load`; catches everything
 *   and returns `-1` on any exception instead of letting it escape the ABI boundary
 * - `congelado_type()` → `T::get_type()`
 * - `congelado_worker_type()` → `T::get_worker_type()`
 * - `congelado_worker_execute(input)` → `T::execute_worker(input)`; `{}` if `s_plugin` was
 *   never constructed or `execute_worker` throws
 * - `congelado_on_unload()` → calls `T::on_unload()`, then deletes and nulls `s_plugin`
 * - `congelado_on_ready()` → `T::on_ready()`
 * - `congelado_on_shutdown()` → `T::on_shutdown_requested()`, a no-op if `s_plugin` was never
 *   constructed
 * - `congelado_on_reload_requested()` → `T::on_reload_requested()` as `1`/`0`
 * - `congelado_call(type, action, args, args_count)` → the universal capability-call ABI,
 *   routed through `_cap_dispatch::call`. `CONGELADO_RUN_LOGGER` + `WRITE`/`ERROR` forwards to
 *   `_cap_dispatch::logger_write` (a no-op if `T` never implements `logger_write`);
 *   `CONGELADO_RUN_STORAGE`/`PROTOCOL`/`SERDE` + `GET` forward to
 *   `_cap_dispatch::storage_get`/`protocol_get`/`serde_get` (`nullptr` if `T` doesn't implement
 *   the matching method) — replaces what used to be 4 separate named C symbols
 *   (`congelado_logger_write(_error)`, `congelado_protocol_get`, `congelado_storage_get`) with
 *   one dlsym'd entrypoint.
 * - `congelado_unique_type()` → `T::get_unique_type()`
 * - `congelado_requires()` / `congelado_requires_count()` → caches `T::get_requires()` into a
 *   static `const char*` array on first call (deque-backed, so the cached pointers stay valid)
 * - `congelado_load_before_types()` / `congelado_load_before_types_count()` → same caching deal
 *   for `T::get_load_before_types()`
 * @param T the `congelado::Plugin` subclass to bridge — must be default-constructible.
 */
#        define CONGELADO_PLUGIN(T) /* NOLINT(cppcoreguidelines-macro-usage) */                    \
            static T* s_plugin =                                                                   \
                nullptr; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */          \
            static std::recursive_mutex s_plugin_mutex; /* NOLINT */                               \
            /* Single guarded accessor: constructs s_plugin once, thread-safely, and never lets   \
             * an exception escape a noexcept extern "C" getter — a failed construction (OOM or   \
             * a throwing ctor) yields nullptr and the getters degrade to ""/0/{} instead of      \
             * std::terminate-ing the host. */                                                     \
            static T* plugin_instance() noexcept /* NOLINT */                                      \
            {                                                                                      \
                try {                                                                              \
                    std::lock_guard<std::recursive_mutex> lock{s_plugin_mutex};                   \
                    if (s_plugin == nullptr) {                                                     \
                        try {                                                                      \
                            s_plugin = new T{};                                                    \
                        } catch (...) {                                                            \
                            s_plugin = nullptr;                                                   \
                        }                                                                          \
                    }                                                                              \
                } catch (...) {                                                                    \
                    return nullptr;                                                                \
                }                                                                                  \
                return s_plugin;                                                                   \
            }                                                                                      \
            extern "C" const char* congelado_plugin_name() noexcept                                \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_name().data() : "";                                              \
            }                                                                                      \
            extern "C" const char* congelado_plugin_version() noexcept                             \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_version().data() : "";                                           \
            }                                                                                      \
            extern "C" uint32_t congelado_capabilities() noexcept                                  \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->capabilities() : 0;                                                  \
            }                                                                                      \
            extern "C" int congelado_init(                                                         \
                const CongeladoHostCallbacks* host, const CongeladoConfigView* cfg                 \
            ) noexcept                                                                             \
            {                                                                                      \
                T* p = plugin_instance();                                                          \
                if (!p) {                                                                          \
                    return -1;                                                                     \
                }                                                                                  \
                try {                                                                              \
                    p->on_load(                                                                    \
                        host ? *host : CongeladoHostCallbacks{},                                   \
                        cfg ? *cfg : CongeladoConfigView{}                                         \
                    );                                                                             \
                } catch (...) {                                                                    \
                    return -1;                                                                     \
                }                                                                                  \
                return 0;                                                                          \
            }                                                                                      \
            extern "C" const char* congelado_type() noexcept                                       \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_type().data() : "";                                              \
            }                                                                                      \
            extern "C" const char* congelado_worker_type() noexcept                                \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_worker_type().data() : "";                                       \
            }                                                                                      \
            extern "C" CongeladoConfigView congelado_worker_execute(                               \
                const CongeladoConfigView* input                                                   \
            ) noexcept                                                                             \
            {                                                                                      \
                T* p = plugin_instance();                                                          \
                if (!p) {                                                                          \
                    return {};                                                                     \
                }                                                                                  \
                try {                                                                              \
                    return p->execute_worker(input);                                               \
                } catch (...) {                                                                    \
                    return {};                                                                     \
                }                                                                                  \
            }                                                                                      \
            /* Lifecycle: expose standardized shared names (no _plugin suffix) */                  \
            extern "C" void congelado_on_unload() noexcept                                         \
            {                                                                                      \
                if (s_plugin != nullptr) {                                                         \
                    try {                                                                          \
                        s_plugin->on_unload();                                                     \
                    } catch (...) {                                                                \
                    }                                                                              \
                    delete s_plugin; /* NOLINT(cppcoreguidelines-owning-memory) */                 \
                    s_plugin = nullptr;                                                            \
                }                                                                                  \
            }                                                                                      \
            extern "C" void congelado_on_ready() noexcept                                          \
            {                                                                                      \
                T* p = plugin_instance();                                                          \
                if (!p) {                                                                          \
                    return;                                                                        \
                }                                                                                  \
                try {                                                                              \
                    p->on_ready();                                                                 \
                } catch (...) {                                                                    \
                }                                                                                  \
            }                                                                                      \
            extern "C" void congelado_on_shutdown() noexcept                                       \
            {                                                                                      \
                T* p = plugin_instance();                                                          \
                if (!p) {                                                                          \
                    return;                                                                        \
                }                                                                                  \
                try {                                                                              \
                    p->on_shutdown_requested();                                                    \
                } catch (...) {                                                                    \
                }                                                                                  \
            }                                                                                      \
            extern "C" int congelado_on_reload_requested() noexcept                                \
            {                                                                                      \
                T* p = plugin_instance();                                                          \
                if (!p) {                                                                          \
                    return 0;                                                                      \
                }                                                                                  \
                try {                                                                              \
                    return p->on_reload_requested() ? 1 : 0;                                       \
                } catch (...) {                                                                    \
                    return 0;                                                                      \
                }                                                                                  \
            }                                                                                      \
            extern "C" CongeladoAny congelado_call(                                                \
                CongeladoRunType type, CongeladoRunAction action, const CongeladoAny* args,        \
                size_t args_count                                                                  \
            ) noexcept                                                                             \
            {                                                                                      \
                T* p = plugin_instance();                                                          \
                if (!p) {                                                                          \
                    return CongeladoAny{};                                                         \
                }                                                                                  \
                return ::congelado::_cap_dispatch::call(p, type, action, args, args_count);        \
            }                                                                                      \
            extern "C" const char* congelado_unique_type() noexcept                                \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_unique_type().data() : "";                                       \
            }                                                                                      \
            extern "C" const char* const* congelado_requires() noexcept                            \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                if (!p) {                                                                          \
                    return nullptr;                                                                \
                }                                                                                  \
                /* deque: push_back keeps references/pointers to existing elements valid, so      \
                 * the cached c_str() pointers survive growth (vector<string> + SSO would not). */ \
                static std::deque<std::string> s_strs; /* NOLINT */                                \
                static std::vector<const char*> s_ptrs; /* NOLINT */                               \
                static bool s_cache_built = false;      /* NOLINT */                               \
                try {                                                                              \
                    std::lock_guard<std::recursive_mutex> lock{s_plugin_mutex};                   \
                    if (!s_cache_built) {                                                          \
                        for (auto sv: p->get_requires()) {                                         \
                            s_strs.emplace_back(sv);                                               \
                            s_ptrs.push_back(s_strs.back().c_str());                               \
                        }                                                                          \
                        s_cache_built = true;                                                      \
                    }                                                                              \
                } catch (...) {                                                                    \
                    return nullptr;                                                                \
                }                                                                                  \
                return s_ptrs.data();                                                              \
            }                                                                                      \
            extern "C" std::size_t congelado_requires_count() noexcept                             \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_requires().size() : 0;                                           \
            }                                                                                      \
            extern "C" const char* const* congelado_load_before_types() noexcept                   \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                if (!p) {                                                                          \
                    return nullptr;                                                                \
                }                                                                                  \
                static std::deque<std::string> s_strs; /* NOLINT */                                \
                static std::vector<const char*> s_ptrs; /* NOLINT */                               \
                static bool s_cache_built = false;      /* NOLINT */                               \
                try {                                                                              \
                    std::lock_guard<std::recursive_mutex> lock{s_plugin_mutex};                   \
                    if (!s_cache_built) {                                                          \
                        for (auto sv: p->get_load_before_types()) {                                \
                            s_strs.emplace_back(sv);                                               \
                            s_ptrs.push_back(s_strs.back().c_str());                               \
                        }                                                                          \
                        s_cache_built = true;                                                      \
                    }                                                                              \
                } catch (...) {                                                                    \
                    return nullptr;                                                                \
                }                                                                                  \
                return s_ptrs.data();                                                              \
            }                                                                                      \
            extern "C" std::size_t congelado_load_before_types_count() noexcept                    \
            {                                                                                      \
                const T* p = plugin_instance();                                                    \
                return p ? p->get_load_before_types().size() : 0;                                  \
            }
#    endif // CONGELADO_PLUGIN_USED

#endif // CONGELADO_GUEST

// NOLINTEND
