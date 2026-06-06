export module worker:context;

import std;
import core_logger;
import io_shared;
import :task_worker;
import :engine_client;

export namespace worker {

// Worker identity + custom task worker registry + engine client.
// Workers are stateless — no database or cache; all persistence lives on the engine side.
//
// set_engine_client() wires the EngineClient (must be called before any handler runs).
// call_engine() blocks the calling thread until the engine responds, using a
// std::promise/std::future pair so the EngineClient's contract can run concurrently
// on another thread in the same ContractThreadPool.
class WorkerContext {
  public:
    using EngineResponseFn = std::function<void(int status, std::string body)>;

    void set_worker_id(std::string_view worker_id) { m_worker_id = worker_id; }

    void set_engine_client(EngineClient &client) noexcept { m_engine_client = &client; }

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

    // Sends an HTTP request to the engine and blocks until the response arrives.
    // Internally uses std::promise/future: the calling thread blocks on future.get()
    // while EngineClient's receive contract runs on another pool thread.
    void call_engine(std::string_view method, std::string_view path, std::string_view body,
                     EngineResponseFn callback) noexcept {
        if (!m_engine_client) {
            core::logger::error("worker/context", "call_engine before engine client is set");
            return;
        }
        auto promise = std::make_shared<std::promise<std::pair<int, std::string>>>();
        auto future = promise->get_future();
        auto req = m_engine_client->make_request<io::shared::http::Protocol>(method, path, body);
        m_engine_client->send<io::shared::http::Protocol>(*req,
            [p = promise](int status, std::string response_body) {
                p->set_value({status, std::move(response_body)});
            });
        try {
            auto [status, response_body] = future.get();
            callback(status, std::move(response_body));
        } catch (...) {
            core::logger::error("worker/context", "engine call failed");
            callback(500, "engine communication error");
        }
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
        auto *worker = get_task_worker(task_type);
        if (worker == nullptr) return std::nullopt;
        auto release  = worker->on_released();
        auto on_error = worker->on_error();
        try {
            auto output = worker->execute(input);
            if (release) release();
            return output;
        } catch (...) {
            if (on_error) on_error(std::current_exception());
            if (release) release();
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
    EngineClient *m_engine_client{nullptr};
    std::string m_worker_id;
    std::vector<ITaskWorker *> m_workers;  // non-owning; TU-statics retain ownership
};

} // namespace worker
