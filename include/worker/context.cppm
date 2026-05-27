export module worker:context;

import std;
import :task_worker;

export namespace worker {

// Worker identity + custom task worker registry.
// Workers are stateless — no database or cache; all persistence lives on the engine side.
class WorkerContext {
  public:
    void set_worker_id(std::string_view worker_id) { m_worker_id = worker_id; }

    // Register a custom worker for a task type. Duplicate task_type replaces previous entry.
    void add_task_worker(std::unique_ptr<ITaskWorker> worker) {
        for (auto &entry : m_workers) {
            if (entry->get_task_type() == worker->get_task_type()) {
                entry = std::move(worker);
                return;
            }
        }
        m_workers.push_back(std::move(worker));
    }

    [[nodiscard]] std::string_view get_worker_id() const noexcept { return m_worker_id; }

    // Returns nullptr if no worker registered for task_type.
    [[nodiscard]] ITaskWorker *get_task_worker(std::string_view task_type) const noexcept {
        for (auto &entry : m_workers) {
            if (entry->get_task_type() == task_type) {
                return entry.get();
            }
        }
        return nullptr;
    }

    // Derived from registered workers — no separate task type list needed.
    [[nodiscard]] std::vector<std::string_view> get_task_types() const noexcept {
        std::vector<std::string_view> types;
        types.reserve(m_workers.size());
        for (auto &entry : m_workers) {
            types.push_back(entry->get_task_type());
        }
        return types;
    }

  private:
    std::string m_worker_id;
    std::vector<std::unique_ptr<ITaskWorker>> m_workers;
};

} // namespace worker
