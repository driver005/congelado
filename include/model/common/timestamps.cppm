export module model:timestamps;

import std;
import serde;

export namespace model {

class ExecutionTimings {
  public:
    ExecutionTimings() = default;

    void set_scheduled_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept {
        m_scheduled_at = tp;
    }
    void set_started_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept {
        m_started_at = tp;
    }
    void set_completed_at(std::optional<std::chrono::system_clock::time_point> tp) noexcept {
        m_completed_at = tp;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_scheduled_at() const noexcept {
        return m_scheduled_at;
    }
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_started_at() const noexcept {
        return m_started_at;
    }
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &
    get_completed_at() const noexcept {
        return m_completed_at;
    }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_scheduled_at && m_started_at && *m_started_at < *m_scheduled_at)
            return std::unexpected{"started_at must not be before scheduled_at"};
        if (m_started_at && m_completed_at && *m_completed_at < *m_started_at)
            return std::unexpected{"completed_at must not be before started_at"};
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
    static constexpr auto fields() {
        return std::tuple{
            serde::field<"scheduled_at", &model::ExecutionTimings::get_scheduled_at,
                       &model::ExecutionTimings::set_scheduled_at>(),
            serde::field<"started_at", &model::ExecutionTimings::get_started_at,
                       &model::ExecutionTimings::set_started_at>(),
            serde::field<"completed_at", &model::ExecutionTimings::get_completed_at,
                       &model::ExecutionTimings::set_completed_at>(),
        };
    }
};
