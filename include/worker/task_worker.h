// NOLINTBEGIN
#pragma once

/* C-compatible factory function type placed in the "congelado_tasks" ELF section.
   Matches the void* return in _congelado_task_create_##T below. */
using CongeladoTaskFactory = void *(*)();

/* Drop exactly once per task class at the bottom of your task .cc file.
   Mirrors CONGELADO_PLUGIN: generates a static singleton + places a factory pointer
   in the "congelado_tasks" linker section. worker_main discovers all compiled-in
   task types by enumerating __start_congelado_tasks / __stop_congelado_tasks.

   Supports both modes:
     Single-task binary: one CONGELADO_TASK → section has 1 entry.
     Bundle binary:      multiple CONGELADO_TASK → section has N entries.

   Usage in a task .cc file:
     import worker;
     #include "worker/task_worker.h"
     class MyTask : public worker::ITaskWorker { ... };
     CONGELADO_TASK(MyTask)

   TODO(macos): replace [[gnu::section("congelado_tasks")]] with
     __attribute__((section("__DATA,congelado_tasks")))
   and use getsectbyname("__DATA", "congelado_tasks") in worker_main.cc
   instead of __start_congelado_tasks / __stop_congelado_tasks. */
#define CONGELADO_TASK(T)  /* NOLINT(cppcoreguidelines-macro-usage) */                               \
    static T *s_task_##T = nullptr; /* NOLINT(cppcoreguidelines-avoid-non-const-global-variables) */ \
    static void *_congelado_task_create_##T() noexcept {                                             \
        if (s_task_##T == nullptr)                                                                   \
            s_task_##T = new T{}; /* NOLINT(cppcoreguidelines-owning-memory) */                      \
        return static_cast<void *>(static_cast<::worker::ITaskWorker *>(s_task_##T));                \
    }                                                                                                 \
    static void _congelado_task_destroy_##T() noexcept {  /* NOLINT */                               \
        if (s_task_##T != nullptr) {                                                                  \
            delete s_task_##T; /* NOLINT(cppcoreguidelines-owning-memory) */                         \
            s_task_##T = nullptr;                                                                     \
        }                                                                                             \
    }                                                                                                 \
    [[gnu::section("congelado_tasks"), gnu::used]]                                                   \
    static CongeladoTaskFactory const _task_factory_##T = &_congelado_task_create_##T; /* NOLINT */
// NOLINTEND
