export module model:timestamps;

import std;
import serde;

export namespace model {

class ExecutionTimings {
  public:
    /// @brief Default ctor — bet, all three timestamps start unset (std::nullopt).
    ExecutionTimings() = default;

    /// @brief Sets when this execution was scheduled to start.
    /// @param value the scheduled time, or std::nullopt to clear it.
    void set_scheduled_at(std::optional<std::chrono::system_clock::time_point> value) noexcept {
        m_scheduled_at = value;
    }
    /// @brief Sets when this execution actually started.
    /// @param value the start time, or std::nullopt to clear it.
    void set_started_at(std::optional<std::chrono::system_clock::time_point> value) noexcept {
        m_started_at = value;
    }
    /// @brief Sets when this execution completed.
    /// @param value the completion time, or std::nullopt to clear it.
    void set_completed_at(std::optional<std::chrono::system_clock::time_point> value) noexcept {
        m_completed_at = value;
    }

    /// @brief Gets when this execution was scheduled to start.
    /// @return the scheduled time, or std::nullopt if it was never set.
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_scheduled_at() const noexcept {
        return m_scheduled_at;
    }
    /// @brief Gets when this execution actually started.
    /// @return the start time, or std::nullopt if it hasn't started yet.
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_started_at() const noexcept {
        return m_started_at;
    }
    /// @brief Gets when this execution completed.
    /// @return the completion time, or std::nullopt if it hasn't completed yet.
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_completed_at() const noexcept {
        return m_completed_at;
    }

    /**
     * @brief Checks that whatever timestamps are set land in scheduled → started → completed
     * order.
     * @warning Only checks pairs where both sides are actually set — if scheduled_at and
     * completed_at are both present but started_at is std::nullopt, there's no direct
     * scheduled-vs-completed check at all, so a completed_at earlier than scheduled_at slips
     * right through. Real gap, not just vibes — don't assume this validates the full chain.
     * @return an empty expected if the ordering holds, otherwise an unexpected describing which
     * pair is out of order.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // First pair: don't let a start time land before the thing was even scheduled.
        if (m_scheduled_at && m_started_at && *m_started_at < *m_scheduled_at) {
            return std::unexpected{"started_at must not be before scheduled_at"};
        }
        // Second pair: same ordering check, but completion can't come before the start — no cap.
        if (m_started_at && m_completed_at && *m_completed_at < *m_started_at) {
            return std::unexpected{"completed_at must not be before started_at"};
        }
        return {};
    }

  private:
    std::optional<std::chrono::system_clock::time_point> m_scheduled_at;
    std::optional<std::chrono::system_clock::time_point> m_started_at;
    std::optional<std::chrono::system_clock::time_point> m_completed_at;
};

} // namespace model

template <>
struct serde::Serializable<model::ExecutionTimings> {
    /**
     * @brief Field-descriptor table wiring ExecutionTimings' scheduled_at/started_at/
     * completed_at to their getters/setters, for serde (de)serialization — no motion needed
     * anywhere else to get this type in/out of JSON or SQL.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"scheduled_at", &model::ExecutionTimings::get_scheduled_at,
                       &model::ExecutionTimings::set_scheduled_at>{},
            serde::FieldDesc<"started_at", &model::ExecutionTimings::get_started_at,
                       &model::ExecutionTimings::set_started_at>{},
            serde::FieldDesc<"completed_at", &model::ExecutionTimings::get_completed_at,
                       &model::ExecutionTimings::set_completed_at>{},
        };
    }
};
