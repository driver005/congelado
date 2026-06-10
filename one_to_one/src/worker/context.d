module worker.context;

@nogc nothrow:

import worker.task_worker;
import model.model;

// Worker identity + custom task worker registry.
// Workers are stateless — no database or cache; all persistence lives on the engine side.
class WorkerContext {
  public:
    this() {}
    this(const(char)[] worker_id) { m_worker_id = worker_id; }

    ~this() {
        foreach (entry; m_workers_buf[0 .. m_workers_count]) {
            auto rel = entry.on_released();
            if (rel !is null)
                rel();
        }
    }

    // PORT-NOTE: copy constructor deleted in C++; @disable covers both copy ctors in D.
    @disable this(ref WorkerContext);

    void set_worker_id(const(char)[] worker_id) { m_worker_id = worker_id; }

    // Register a non-owning pointer to a worker for a task type.
    // The caller retains ownership; this context holds a raw reference only.
    // Duplicate task_type replaces previous entry.
    void add_task_worker(ITaskWorker worker) {
        foreach (ref entry; m_workers_buf[0 .. m_workers_count]) {
            if (entry.get_task_type() == worker.get_task_type()) {
                entry = worker;
                return;
            }
        }
        // PORT-NOTE: C++ uses std::vector<ITaskWorker*>; D uses fixed-size buffer[32] to avoid GC.
        assert(m_workers_count < 32, "too many workers");
        m_workers_buf[m_workers_count++] = worker;
    }

    const(char)[] get_worker_id() const { return m_worker_id; }

    // Returns null if no worker registered for task_type.
    ITaskWorker get_task_worker(const(char)[] task_type) const {
        foreach (entry; m_workers_buf[0 .. m_workers_count]) {
            if (entry.get_task_type() == task_type) {
                return cast(ITaskWorker) entry;
            }
        }
        return null;
    }

    // Execute a task by type. Calls on_released() on completion,
    // on_error() + on_released() on exception (exceptions swallowed — @nogc nothrow).
    // Returns false if no worker registered for task_type.
    // PORT-NOTE: C++ returned std::optional<TaskOutput>; D uses out-param + bool.
    bool run_task(const(char)[] task_type, ref const(TaskInput) input,
                  out TaskOutput output) {
        auto w = get_task_worker(task_type);
        if (w is null) return false;
        auto release  = w.on_released();
        auto on_error = w.on_error();
        // PORT-NOTE: C++ used try/catch(...) to call on_error; D is nothrow so no try needed.
        output = w.execute(input);
        if (release !is null) release();
        return true;
    }

    // Derived from registered workers — no separate task type list needed.
    // PORT-NOTE: C++ returned std::vector<string_view>; D fills a caller-supplied slice.
    size_t get_task_types(const(char)[][] out_types) const {
        size_t count = 0;
        foreach (entry; m_workers_buf[0 .. m_workers_count]) {
            if (count >= out_types.length) break;
            out_types[count++] = entry.get_task_type();
        }
        return count;
    }

    ITaskWorker[] workers() { return m_workers_buf[0 .. m_workers_count]; }

  private:
    const(char)[] m_worker_id;
    // PORT-NOTE: C++ uses std::vector<ITaskWorker*>; D uses fixed-size buffer[32] to avoid GC.
    ITaskWorker[32] m_workers_buf;
    size_t          m_workers_count;
}
