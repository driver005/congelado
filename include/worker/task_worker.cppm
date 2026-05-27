export module worker:task_worker;

import std;

export namespace worker {

// Opaque input bag passed to ITaskWorker::execute.
// input_data is a JSON blob from the TaskInstance input_params field.
class TaskInput {
  public:
    void set_task_id(std::string_view task_id) { m_task_id = task_id; }
    void set_task_type(std::string_view task_type) { m_task_type = task_type; }
    void set_input_data(std::string_view input_data) { m_input_data = input_data; }

    [[nodiscard]] std::string_view get_task_id() const noexcept { return m_task_id; }
    [[nodiscard]] std::string_view get_task_type() const noexcept { return m_task_type; }
    [[nodiscard]] std::string_view get_input_data() const noexcept { return m_input_data; }

  private:
    std::string m_task_id;
    std::string m_task_type;
    std::string m_input_data;
};

// Opaque output bag filled by ITaskWorker::execute.
// output_data is a JSON blob forwarded to the engine as the TaskResult output_data field.
class TaskOutput {
  public:
    void set_output_data(std::string_view output_data) { m_output_data = output_data; }
    void set_success(bool success) noexcept { m_success = success; }

    [[nodiscard]] std::string_view get_output_data() const noexcept { return m_output_data; }
    [[nodiscard]] bool get_success() const noexcept { return m_success; }

  private:
    std::string m_output_data;
    bool m_success{false};
};

// Implement this once per custom task type and register via WorkerContext::add_task_worker.
//
// Example:
//   class MySendEmailWorker : public worker::ITaskWorker {
//   public:
//       std::string_view get_task_type() const noexcept override { return "SEND_EMAIL"; }
//       void execute(const worker::TaskInput& input, worker::TaskOutput& output) noexcept override {
//           // parse input.get_input_data(), send email, fill output
//           output.set_output_data(R"({"sent":true})");
//           output.set_success(true);
//       }
//   };
//   ctx.add_task_worker(std::make_unique<MySendEmailWorker>());
class ITaskWorker {
  public:
    virtual ~ITaskWorker() = default;
    ITaskWorker() = default;
    ITaskWorker(const ITaskWorker &) = delete;
    ITaskWorker &operator=(const ITaskWorker &) = delete;
    ITaskWorker(ITaskWorker &&) = delete;
    ITaskWorker &operator=(ITaskWorker &&) = delete;

    // Must return the same string literal every call — stored as a key in the worker registry.
    [[nodiscard]] virtual std::string_view get_task_type() const noexcept = 0;

    // Execute the task. Must not throw. Set output.set_success(false) on failure.
    virtual void execute(const TaskInput &input, TaskOutput &output) noexcept = 0;
};

} // namespace worker
