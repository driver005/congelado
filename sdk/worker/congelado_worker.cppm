export module congelado_worker;

export import worker:task_worker;
import std;

export namespace congelado {

using TaskInput  = worker::TaskInput;
using TaskOutput = worker::TaskOutput;

class ITask : public worker::ITaskWorker {
  public:
    [[nodiscard]] virtual std::string_view type() const noexcept = 0;
    [[nodiscard]] virtual TaskOutput       run(TaskInput const &)  = 0;

  private:
    [[nodiscard]] std::string_view get_task_type() const noexcept final { return type(); }
    [[nodiscard]] TaskOutput       execute(TaskInput const &in)   final { return run(in); }
};

namespace detail {

class TaskRegistry {
  public:
    using Factory = std::function<ITask *()>;

    static TaskRegistry &instance() noexcept {
        static TaskRegistry s_instance;
        return s_instance;
    }

    // Registration is called only from static-init (CONGELADO_TASK macros), which runs
    // single-threaded before main(). Not safe to call concurrently.
    void register_task(Factory factory) {
        std::unique_ptr<ITask> instance(factory());
        if (instance == nullptr) return; // NOLINT: should never happen with CONGELADO_TASK
        auto key = std::string(instance->type());
        m_tasks.emplace(std::move(key), std::move(instance));
    }

    [[nodiscard]] std::vector<ITask *> all() const {
        std::vector<ITask *> result;
        result.reserve(m_tasks.size());
        for (auto const &[_, task] : m_tasks)
            result.push_back(task.get());
        return result;
    }

  private:
    std::unordered_map<std::string, std::unique_ptr<ITask>> m_tasks;

    TaskRegistry()  = default;
};

} // namespace detail
} // namespace congelado
