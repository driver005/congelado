// NOLINTBEGIN
#pragma once
#include <congelado/abi.h>

#include <mutex>

#ifdef __cplusplus

/* CONGELADO_TASK(T): define a per-TU worker instance and emit C ABI symbols
   for the worker lifecycle and execution.

   The class T must provide:
     std::string_view get_worker_type() const noexcept;
     CongeladoConfigView execute_worker(const CongeladoConfigView *input);

   The macro generates:
     congelado_type()             → "worker"
     congelado_worker_type()      → T::get_worker_type()
     congelado_worker_execute()   → T::execute_worker()
     congelado_init()             → create T instance
     congelado_on_unload()        → destroy T instance

   Notes:
   - MUST be used exactly once per translation unit (enforced by guard).
   - Do not put in headers; place in .cc/.cpp after the class definition.
   - Cannot coexist with CONGELADO_PLUGIN in the same TU.
*/

#    if defined(CONGELADO_PLUGIN_USED)
#        error                                                                                     \
            "CONGELADO_TASK cannot be used in the same translation unit as CONGELADO_PLUGIN; move task definitions to a separate file."
#    endif

#    ifndef CONGELADO_TASK_USED
#        define CONGELADO_TASK_USED
/**
 * @def CONGELADO_TASK(T)
 * @brief Drops a lazily-constructed, TU-local `static T *s_inst` (inside an anonymous
 * namespace, so no symbol leaks across TUs) plus every `extern "C"` symbol the host dlsym's off
 * a worker `.so` — the whole bridge from a pure-C++ task class to the `CONGELADO_TASK` C ABI.
 * @warning MUST be used exactly once per translation unit (enforced by the
 * `CONGELADO_TASK_USED` guard — a second invocation in the same TU hits the `#error` below), and
 * it cannot coexist with `CONGELADO_PLUGIN` in that same TU either. Drop it in a `.cc`/`.cpp`
 * after `T`'s definition, never in a header — that's an easy mistake for a first-time worker
 * author to make, and it's a straight L (ODR violations across every TU that includes it).
 * @details Generated C symbols:
 * - `congelado_type()` → always returns `"worker"`
 * - `congelado_worker_type()` → constructs `s_inst` if needed, then `T::get_worker_type()`
 * - `congelado_worker_execute(input)` → `T::execute_worker(input)`; no-op `{}` if `s_inst` was
 *   never constructed
 * - `congelado_init(host, cfg)` → constructs `s_inst` if needed; `host`/`cfg` are unused by this
 *   macro (workers don't get host callbacks the way plugins do), wrapped in try/catch that
 *   returns `-1` on any exception instead of letting it cross the ABI boundary
 * - `congelado_on_unload()` → deletes and nulls `s_inst`
 * @param T the worker task class to bridge — must be default-constructible and provide
 * `std::string_view get_worker_type() const noexcept` plus
 * `CongeladoConfigView execute_worker(const CongeladoConfigView *input)`, per the requirements
 * documented above.
 */
#        define CONGELADO_TASK(T)                                                                  \
            namespace {                                                                            \
            static T* s_inst = nullptr;                                                            \
            /* Guarded accessor: thread-safe lazy construction that never lets an exception      \
             * escape a noexcept extern "C" symbol — failed construction (OOM or a throwing       \
             * ctor) yields nullptr and the getters degrade to ""/{} instead of terminate.       */ \
            static T* task_instance() noexcept                                                    \
            {                                                                                      \
                static std::recursive_mutex m; /* NOLINT */                                       \
                try {                                                                              \
                    std::lock_guard<std::recursive_mutex> lock{m};                                \
                    if (s_inst == nullptr) {                                                       \
                        try {                                                                      \
                            s_inst = new T{};                                                      \
                        } catch (...) {                                                            \
                            s_inst = nullptr;                                                      \
                        }                                                                          \
                    }                                                                              \
                } catch (...) {                                                                    \
                    return nullptr;                                                                \
                }                                                                                  \
                return s_inst;                                                                     \
            }                                                                                      \
            }                                                                                      \
            extern "C" const char* congelado_type() noexcept                                       \
            {                                                                                      \
                return "worker";                                                                   \
            }                                                                                      \
            extern "C" const char* congelado_worker_type() noexcept                                \
            {                                                                                      \
                const T* p = task_instance();                                                      \
                return p ? p->get_worker_type().data() : "";                                       \
            }                                                                                      \
            extern "C" CongeladoConfigView congelado_worker_execute(                               \
                const CongeladoConfigView* input                                                   \
            ) noexcept                                                                             \
            {                                                                                      \
                T* p = task_instance();                                                            \
                if (!p) {                                                                          \
                    return {};                                                                     \
                }                                                                                  \
                try {                                                                              \
                    return p->execute_worker(input);                                               \
                } catch (...) {                                                                    \
                    return {};                                                                     \
                }                                                                                  \
            }                                                                                      \
            extern "C" int congelado_init(                                                         \
                const CongeladoHostCallbacks* host, const CongeladoConfigView* cfg                 \
            ) noexcept                                                                             \
            {                                                                                      \
                (void)host;                                                                        \
                (void)cfg;                                                                         \
                return task_instance() != nullptr ? 0 : -1;                                        \
            }                                                                                      \
            extern "C" void congelado_on_unload() noexcept                                         \
            {                                                                                      \
                delete s_inst;                                                                     \
                s_inst = nullptr;                                                                  \
            }
#    else
#        error "Only one CONGELADO_TASK macro invocation is allowed per translation unit."
#    endif // CONGELADO_TASK_USED

#endif // __cplusplus
// NOLINTEND
