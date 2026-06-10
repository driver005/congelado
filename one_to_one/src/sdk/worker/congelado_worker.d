module sdk.worker.congelado_worker;

// PORT-NOTE: C++ used std::function<ITask*()> Factory and std::unordered_map.
// D uses a fixed factory table (max 64 task types) to avoid GC.
// TaskRegistry singleton uses __gshared + static construction.

import worker.task_worker : ITaskWorker, TaskInput, TaskOutput;
import util.optional      : Optional;

// ─── congelado.TaskInput / TaskOutput aliases ─────────────────────────────────

alias CongeladoTaskInput  = TaskInput;
alias CongeladoTaskOutput = TaskOutput;

// ─── congelado.ITask ──────────────────────────────────────────────────────────

abstract class ITask : ITaskWorker {
  public:
    abstract const(char)[]     get_type()           const nothrow;
    abstract CongeladoTaskOutput run(const ref CongeladoTaskInput input);
    void                       release()             nothrow {}
    void                       error(void* /*exception_ptr*/) nothrow {}

  private:
    // Implement ITaskWorker interface by delegating to abstract methods above.
    override final const(char)[] get_task_type() const nothrow { return get_type(); }
    override final CongeladoTaskOutput execute(const ref CongeladoTaskInput input) {
        return run(input);
    }
    override final void function() @nogc nothrow on_released() nothrow {
        return null; // PORT-NOTE: C++ returned lambda calling release(); stub here.
    }
    override final void function(void*) @nogc nothrow on_error() nothrow {
        return null; // PORT-NOTE: C++ returned lambda calling error(); stub here.
    }
}

// ─── detail.TaskRegistry ──────────────────────────────────────────────────────

// PORT-NOTE: C++ used std::unordered_map<std::string, std::unique_ptr<ITask>>.
// D uses a fixed table of 64 slots. Registration runs in static init (single-threaded).

private struct TaskSlot {
    ITask task;
    bool  occupied;
}

class CongeladoTaskRegistry {
  public:
    alias Factory = ITask function() @nogc nothrow;

    // Singleton.
    // Registration is called only from static-init (CongeladoTask mixin), which runs
    // single-threaded before main(). Not safe to call concurrently.
    static CongeladoTaskRegistry instance() nothrow {
        if (s_instance is null)
            s_instance = new CongeladoTaskRegistry();
        return s_instance;
    }

    void add_task_factory(Factory factory) nothrow {
        // PORT-NOTE: C++ created an instance to get the type key.
        // D does the same via factory().
        if (m_count >= 64) return;
        auto inst = factory();
        if (inst is null) return;
        m_slots[m_count++] = TaskSlot(inst, true);
    }

    // Returns all registered task instances (non-owning view).
    // PORT-NOTE: C++ returned std::vector<ITask*>; D fills caller-supplied slice.
    size_t get_all(ITask[] out_buf) const nothrow {
        size_t n = 0;
        foreach (ref slot; m_slots[0 .. m_count]) {
            if (!slot.occupied) continue;
            if (n >= out_buf.length) break;
            out_buf[n++] = cast(ITask) slot.task;
        }
        return n;
    }

  private:
    this() nothrow {}

    TaskSlot[64]               m_slots;
    size_t                     m_count;
    __gshared CongeladoTaskRegistry s_instance = null;
}
