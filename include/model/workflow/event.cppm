export module model:workflow_event;

import std;
import :identifiers;
import serde;

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
    void set_issued_at(std::chrono::system_clock::time_point tp) noexcept { m_issued_at = tp; }

    [[nodiscard]] WorkflowEventType get_type() const noexcept { return m_type; }
    [[nodiscard]] const ExecutionId &get_exec_id() const noexcept { return m_exec_id; }
    [[nodiscard]] const std::optional<std::string> &get_payload() const noexcept {
        return m_payload;
    }
    [[nodiscard]] const std::chrono::system_clock::time_point &get_issued_at() const noexcept {
        return m_issued_at;
    }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_exec_id == ExecutionId{})
            return std::unexpected{"WorkflowEvent exec_id must not be nil"};
        return {};
    }

  private:
    ExecutionId m_exec_id;
    WorkflowEventType m_type{};
    std::optional<std::string> m_payload;
    std::chrono::system_clock::time_point m_issued_at;
};

} // namespace model

template <>
struct serde::Serializable<model::WorkflowEvent> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"exec_id", &model::WorkflowEvent::get_exec_id,
                       &model::WorkflowEvent::set_exec_id>{},
            serde::FieldDesc<"type", &model::WorkflowEvent::get_type, &model::WorkflowEvent::set_type>{},
            serde::FieldDesc<"payload", &model::WorkflowEvent::get_payload,
                       &model::WorkflowEvent::set_payload>{},
            serde::FieldDesc<"issued_at", &model::WorkflowEvent::get_issued_at,
                       &model::WorkflowEvent::set_issued_at>{},
        };
    }
};
