// NOLINTBEGIN
#pragma once
#include <congelado/abi.h>

#ifdef __cplusplus
namespace congelado {

} // namespace congelado

/* CONGELADO_TASK(T): define a single per-translation-unit task instance and
   emit the worker lifecycle symbols that operate on that instance.

   Notes:
   - The macro MUST be used exactly once per translation unit (enforced by the
     CONGELADO_TASK_USED guard). This prevents duplicate symbol definitions.
   - The macro does NOT register the task in any global registry. The TU owns
     the task instance and is responsible for its lifetime (the macro deletes
     the instance on unload).
   - Initialization now happens lazily inside congelado_init(), not at static
     initialization time. This avoids static-init order issues.
   - Do not put CONGELADO_TASK in headers; place it in a .cc/.cpp file after
     the task class definition.
*/

#if defined(CONGELADO_PLUGIN_USED)
#error "CONGELADO_TASK cannot be used in the same translation unit as CONGELADO_PLUGIN; move task definitions to a separate file."
#endif

#ifndef CONGELADO_TASK_USED
#define CONGELADO_TASK_USED
#define CONGELADO_TASK(T)                                                                          \
    namespace { /* anonymous TU-local storage */                                                    \
    static T *s_task_##T = nullptr; /* per-TU owned task instance */                                \
    }                                                                                             \
    /* Export standard worker lifecycle C symbols (one set per TU that uses the macro) */          \
    extern "C" int congelado_init(const CongeladoHostCallbacks *host,                             \
                                   const CongeladoConfigView *cfg) noexcept {                     \
        (void)host; (void)cfg;                                                                     \
        try {                                                                                      \
            if (s_task_##T == nullptr)                                                             \
                s_task_##T = new T{}; /* NOLINT(cppcoreguidelines-owning-memory) */                  \
        } catch (...) {                                                                            \
            return -1;                                                                             \
        }                                                                                          \
        return 0;                                                                                  \
    }                                                                                            \
    extern "C" void congelado_on_ready() noexcept {                                               \
        if (s_task_##T != nullptr) {                                                              \
            try { s_task_##T->on_ready(); } catch (...) { }                                        \
        }                                                                                         \
    }                                                                                            \
    extern "C" void congelado_on_unload() noexcept {                                              \
        if (s_task_##T != nullptr) {                                                              \
            try { s_task_##T->on_unload(); } catch (...) { }                                       \
            delete s_task_##T; /* NOLINT(cppcoreguidelines-owning-memory) */                        \
            s_task_##T = nullptr;                                                                 \
        }                                                                                         \
    }                                                                                            \
    extern "C" int congelado_on_reload_requested() noexcept {                                     \
        if (s_task_##T == nullptr) return 1;                                                      \
        try {                                                                                     \
            return s_task_##T->on_reload_requested() ? 1 : 0;                                      \
        } catch (...) { return 0; }                                                               \
    }
#else
#error "Only one CONGELADO_TASK macro invocation is allowed per translation unit. Move additional tasks to separate files or use a module-level registration macro."
#endif // CONGELADO_TASK_USED

#endif // __cplusplus
// NOLINTEND
