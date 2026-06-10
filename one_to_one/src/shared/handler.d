module shared.handler;
@nogc nothrow:

// PORT-NOTE: std::function<void()> → void function(void*) @nogc nothrow + void* ctx,
// because @nogc forbids delegates with GC closures. The ctx pointer carries the
// closure state that the C++ std::function captures.
//
// For HandlerInterface and HandlerBase, virtual methods that throw in C++
// (e.g. shedule/deschedule/release in this_handler) become nothrow; the error
// path is a no-op or returns an error code instead.

// PORT-NOTE: std::move_only_function<void()> at call sites
alias WorkerFunction  = void function(void* ctx) @nogc nothrow;
alias ReleaseFunction = void function(void* ctx) @nogc nothrow;
alias ErrorHandler    = void function(void* ctx, void* exception_info) @nogc nothrow;

// HandlerTemplate concept — expressed as a D template constraint.
template HandlerTemplate(T) {
    enum bool HandlerTemplate = is(typeof({
        T* t;
        t.schedule();
        t.deschedule();
        t.release();
    }));
}

extern(C++) interface HandlerInterface {
    void schedule(uint id);
    void deschedule(uint id);
    void release(uint id);
}

// HandlerController concept — expressed as a D template constraint.
// Mirrors the C++ HandlerController which requires schedule/deschedule/release(id)
// plus a create(name, work, release, error, opt_args...) that returns a HandlerTemplate.
template HandlerController(T) {
    enum bool HandlerController = is(typeof({
        T* a;
        a.schedule(0u);
        a.deschedule(0u);
        a.release(0u);
        // create() is checked structurally at use-site
    }));
}

// Forward declaration (defined below)
class HandlerBase;

// ExecutionPattern concept
template ExecutionPattern(T, Args...) {
    enum bool ExecutionPattern = is(typeof({
        // T::install(handler, args...) must be callable
    }));
}

abstract class HandlerBase {
    abstract const(char)[] get_name() const @nogc nothrow;

    abstract WorkerFunction  on_execute() @nogc nothrow;

    auto create(TController, Args...)(ref TController controller, auto ref Args args) @nogc nothrow
        if (HandlerController!TController)
    {
        return controller.create(get_name(), on_execute(), on_released(), on_error(), args);
    }

    ReleaseFunction on_released() @nogc nothrow { return null; }

    ErrorHandler on_error() @nogc nothrow { return null; }
}

// this_handler — thread-local scheduler context
// PORT-NOTE: thread_local is supported in D via __gshared + TLS or plain TLS vars.
// The throw-on-null safety checks from C++ are preserved as no-ops (nothrow constraint).
// Callers must ensure `current` is non-null before calling schedule/deschedule/release.

struct this_handler {
@nogc nothrow:
    static HandlerInterface* current;        // thread_local in D = default for module-level
    static uint current_id = uint.max;

    static void shedule() {
        if (current is null) return; // no context — silently ignore (nothrow)
        current.schedule(current_id);
    }

    static void deschedule() {
        if (current is null) return;
        current.deschedule(current_id);
    }

    static void release() {
        if (current is null) return;
        current.release(current_id);
    }
}
