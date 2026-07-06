export module worker:context;

import std;
import core_logger;
import core_client;
import io_shared;
import :task_worker;

export namespace worker {

// Worker identity + custom task worker registry.
// Workers are stateless — no database or cache; all persistence lives on the engine side.
class WorkerContext {
  public:
    WorkerContext() = default;
    explicit WorkerContext(std::string_view worker_id) : m_worker_id(worker_id) {}

    ~WorkerContext() {
        for (auto *entry : m_workers) {
            if (entry->on_released())
                entry->on_released()();
        }
    }

    WorkerContext(WorkerContext const &) = delete;

    WorkerContext &operator=(WorkerContext const &) = delete;
    WorkerContext(WorkerContext &&) = default;
    WorkerContext &operator=(WorkerContext &&) = default;

    void set_worker_id(std::string_view worker_id) { m_worker_id = worker_id; }

    // Register a non-owning pointer to a worker for a task type.
    // The caller retains ownership; this context holds a raw reference only.
    // Duplicate task_type replaces previous entry.
    void add_task_worker(ITaskWorker *worker) {
        for (auto &entry : m_workers) {
            if (entry->get_task_type() == worker->get_task_type()) {
                entry = worker;
                return;
            }
        }
        m_workers.push_back(worker);
    }

    [[nodiscard]] std::string_view get_worker_id() const noexcept { return m_worker_id; }

    // Returns nullptr if no worker registered for task_type.
    [[nodiscard]] ITaskWorker *get_task_worker(std::string_view task_type) const noexcept {
        for (auto *entry : m_workers) {
            if (entry->get_task_type() == task_type) {
                return entry;
            }
        }
        return nullptr;
    }

    // Execute a task by type. Calls release() on completion, error() + release() on exception.
    // Returns nullopt if no worker registered for task_type.
    [[nodiscard]] std::optional<TaskOutput> run_task(std::string_view task_type,
                                                     TaskInput const &input) {
        auto *w = get_task_worker(task_type);
        if (w == nullptr)
            return std::nullopt;
        auto release = w->on_released();
        auto on_error = w->on_error();
        try {
            auto output = w->execute(input);
            if (release)
                release();
            return output;
        } catch (...) {
            if (on_error)
                on_error(std::current_exception());
            if (release)
                release();
            return std::nullopt;
        }
    }

    // Derived from registered workers — no separate task type list needed.
    [[nodiscard]] std::vector<std::string_view> get_task_types() const noexcept {
        std::vector<std::string_view> types;
        types.reserve(m_workers.size());
        for (auto *entry : m_workers) {
            types.push_back(entry->get_task_type());
        }
        return types;
    }

  private:
    std::string m_worker_id;
    std::vector<ITaskWorker *> m_workers;
};

} // namespace worker
