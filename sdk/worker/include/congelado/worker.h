// NOLINTBEGIN
#pragma once
#include <congelado/abi.h>

#ifdef __cplusplus
namespace congelado {
int run_worker(int argc, char **argv);
} // namespace congelado

/* CONGELADO_TASK(T): register a congelado::ITask subclass with the worker runtime.
   T must publicly inherit congelado::ITask. Drop exactly once per task class
   at file scope in the task .cc file, after the class definition.
   Requires: import congelado_worker; (provides congelado::ITask and TaskRegistry). */
#define CONGELADO_TASK(T)                                                                          \
    namespace { /* NOLINT(cert-dcl59-cpp) */                                                      \
    [[maybe_unused]] bool const _congelado_registered_##T = []() -> bool {              \
        congelado::detail::TaskRegistry::instance().add_task(                                \
            []() -> congelado::ITask * { return new T{}; } /* NOLINT */                           \
        );                                                                                        \
        return true;                                                                              \
    }();                                                                                          \
    } /* anonymous namespace */                                                                   \
    /* Emit worker lifecycle exports (one task per TU only) */                                   \
    extern "C" int congelado_init(const CongeladoHostCallbacks *host,                          \
                                   const CongeladoConfigView *cfg) noexcept {                    \
        (void)host; (void)cfg; /* Worker init: no-op by default */                               \
        return 0;                                                                                 \
    }                                                                                            \
    extern "C" void congelado_on_ready() noexcept {                                             \
        /* Worker ready: no-op by default */                                                      \
    }                                                                                            \
    extern "C" void congelado_on_unload() noexcept {                                            \
        /* Worker unload: no-op by default */                                                     \
    }                                                                                            \
    extern "C" int congelado_on_reload_requested() noexcept {                                   \
        return 1; /* allow reloads by default */                                                 \
    }

#endif // __cplusplus
// NOLINTEND
