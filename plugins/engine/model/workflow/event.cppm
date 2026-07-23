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
    /// @brief Default ctor, bet — nil exec_id, type value-initialized to PAUSE (enumerator 0),
    /// no payload, issued_at at the epoch. Set exec_id before this'll validate.
    WorkflowEvent() = default;

    /// @brief Sets which workflow execution this event targets.
    /// @param execution_id the target workflow execution's id.
    void set_exec_id(ExecutionId execution_id) { m_exec_id = execution_id; }
    /// @brief Sets the event type.
    /// @param type PAUSE, RESUME, TERMINATE, RESTART, or SIGNAL.
    void set_type(WorkflowEventType type) noexcept { m_type = type; }
    /// @brief Sets the event's optional payload.
    /// @param payload the payload data, or std::nullopt for events that carry none.
    void set_payload(std::optional<std::string> payload) { m_payload = std::move(payload); }
    /// @brief Sets when this event was issued.
    /// @param timestamp the issuance timestamp.
    void set_issued_at(std::chrono::system_clock::time_point timestamp) noexcept {
        m_issued_at = timestamp;
    }

    /// @brief Gets the event type.
    /// @return PAUSE, RESUME, TERMINATE, RESTART, or SIGNAL.
    [[nodiscard]] WorkflowEventType get_type() const noexcept { return m_type; }
    /// @brief Gets which workflow execution this event targets.
    /// @return the target workflow execution's id.
    [[nodiscard]] const ExecutionId &get_exec_id() const noexcept { return m_exec_id; }
    /// @brief Gets the event's optional payload.
    /// @return the payload data, or std::nullopt if this event carries none.
    [[nodiscard]] const std::optional<std::string> &get_payload() const noexcept {
        return m_payload;
    }
    /// @brief Gets when this event was issued.
    /// @return the issuance timestamp.
    [[nodiscard]] const std::chrono::system_clock::time_point &get_issued_at() const noexcept {
        return m_issued_at;
    }

    /**
     * @brief Checks that exec_id actually targets a real execution.
     * @warning Only checks exec_id nil-ness — a SIGNAL event with no payload sails through
     * clean even though a signal with nothing attached is lowkey pointless. If payload is
     * required for a given type, that's not enforced here.
     * @return an empty expected if exec_id is non-nil, otherwise an unexpected explaining why.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_exec_id == ExecutionId{}) {
            return std::unexpected{"WorkflowEvent exec_id must not be nil"};
        }
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
    /**
     * @brief Field-descriptor table wiring WorkflowEvent's exec_id/type/payload/issued_at to
     * their getters/setters, for serde (de)serialization — no motion happens on the wire
     * without this table.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
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
