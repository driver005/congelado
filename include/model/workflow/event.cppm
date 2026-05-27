export module model:workflow_event;

import std;
import :identifiers;

export namespace model {

enum class WorkflowEventType : std::uint8_t {
    PAUSE,
    RESUME,
    TERMINATE,
    RESTART,
    SIGNAL,
};

class WorkflowEvent {
  public:
    WorkflowEvent() = default;

    void set_exec_id(ExecutionId execution_id) { m_exec_id = execution_id; }
    void set_type(WorkflowEventType type) noexcept { m_type = type; }
    void set_payload(std::optional<std::string> payload) { m_payload = std::move(payload); }
    void set_issued_at(std::chrono::system_clock::time_point issued_at) noexcept { m_issued_at = issued_at; }

    [[nodiscard]] WorkflowEventType get_type() const noexcept { return m_type; }
    [[nodiscard]] const ExecutionId &get_exec_id() const noexcept { return m_exec_id; }
    [[nodiscard]] const std::optional<std::string> &get_payload() const noexcept { return m_payload; }
    [[nodiscard]] const std::chrono::system_clock::time_point &get_issued_at() const noexcept { return m_issued_at; }

  private:
    ExecutionId m_exec_id;
    WorkflowEventType m_type{};
    std::optional<std::string> m_payload;
    std::chrono::system_clock::time_point m_issued_at;
};

} // namespace model
