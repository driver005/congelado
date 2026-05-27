module;

export module model:timestamps;

import std;

export namespace model {

class ExecutionTimings {
  public:
    ExecutionTimings() = default;

    void set_scheduled_at(std::chrono::system_clock::time_point scheduled_at) noexcept {
        m_scheduled_at = scheduled_at;
    }
    void set_started_at(std::chrono::system_clock::time_point started_at) noexcept { m_started_at = started_at; }
    void set_completed_at(std::chrono::system_clock::time_point completed_at) noexcept {
        m_completed_at = completed_at;
    }

    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &scheduled_at() const noexcept {
        return m_scheduled_at;
    }
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &started_at() const noexcept {
        return m_started_at;
    }
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point> &completed_at() const noexcept {
        return m_completed_at;
    }

  private:
    std::optional<std::chrono::system_clock::time_point> m_scheduled_at;
    std::optional<std::chrono::system_clock::time_point> m_started_at;
    std::optional<std::chrono::system_clock::time_point> m_completed_at;
};

} // namespace model
