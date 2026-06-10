module sdk.worker.worker;

// PORT-NOTE: SDK C header — uses -betterC/extern(C) rules only.
// No Object base, no exceptions, only extern(C) struct and extern(C) functions.

// CONGELADO_TASK(T) macro is translated to a mixin template CongeladoTask!T
// that performs the equivalent static-init registration.

extern(C++) int congelado_run_worker(int argc, char** argv) @nogc nothrow;

// Mixin template equivalent of CONGELADO_TASK(T) macro.
// Drop at module scope in each task file after the class definition:
//   mixin CongeladoTask!MyTaskClass;
//
// PORT-NOTE: C++ used a static bool initializer with anonymous namespace + lambda.
// D uses a shared static this() in the mixin, which runs at module load time.
mixin template CongeladoTask(T) {
    // Registered at module startup.
    // congelado.detail.TaskRegistry is defined in congelado_worker.d.
    shared static this() {
        import sdk.worker.congelado_worker : CongeladoTaskRegistry;
        CongeladoTaskRegistry.instance().add_task_factory(() => new T());
    }
}
