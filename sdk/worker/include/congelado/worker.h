// NOLINTBEGIN
#pragma once

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
        congelado::detail::TaskRegistry::instance().register_task(                                \
            []() -> congelado::ITask * { return new T{}; } /* NOLINT */                           \
        );                                                                                        \
        return true;                                                                              \
    }();                                                                                          \
    } /* anonymous namespace */

#endif // __cplusplus
// NOLINTEND
