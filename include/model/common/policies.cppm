export module model:policies;

import std;
import ser;

export namespace model {

enum class RetryBackoff : std::uint8_t { FIXED, EXPONENTIAL };

class RetryPolicy {
  public:
    RetryPolicy(std::uint32_t max_attempts = 3, RetryBackoff backoff = RetryBackoff::FIXED,
                std::uint32_t interval_ms = 1000)
        : m_max_attempts{max_attempts}, m_backoff{backoff}, m_interval_ms{interval_ms} {}

    void set_max_attempts(std::uint32_t max_attempts) noexcept { m_max_attempts = max_attempts; }
    void set_backoff(RetryBackoff backoff) noexcept { m_backoff = backoff; }
    void set_interval_ms(std::uint32_t interval_ms) noexcept { m_interval_ms = interval_ms; }

    [[nodiscard]] std::uint32_t get_max_attempts() const noexcept { return m_max_attempts; }
    [[nodiscard]] RetryBackoff get_backoff() const noexcept { return m_backoff; }
    [[nodiscard]] std::uint32_t get_interval_ms() const noexcept { return m_interval_ms; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_max_attempts == 0) return std::unexpected{"max_attempts must be at least 1"};
        if (m_interval_ms == 0)  return std::unexpected{"interval_ms must be greater than 0"};
        return {};
    }

  private:
    std::uint32_t m_max_attempts;
    RetryBackoff m_backoff;
    std::uint32_t m_interval_ms;
};

enum class TimeoutAction : std::uint8_t { RETRY, FAIL_WORKFLOW, ALERT_ONLY };

class TimeoutPolicy {
  public:
    TimeoutPolicy(std::uint32_t timeout_ms = 30000, TimeoutAction action = TimeoutAction::FAIL_WORKFLOW)
        : m_timeout_ms{timeout_ms}, m_action{action} {}

    void set_timeout_ms(std::uint32_t timeout_ms) noexcept { m_timeout_ms = timeout_ms; }
    void set_action(TimeoutAction action) noexcept { m_action = action; }

    [[nodiscard]] std::uint32_t get_timeout_ms() const noexcept { return m_timeout_ms; }
    [[nodiscard]] TimeoutAction get_action() const noexcept { return m_action; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_timeout_ms == 0) return std::unexpected{"timeout_ms must be greater than 0"};
        return {};
    }

  private:
    std::uint32_t m_timeout_ms;
    TimeoutAction m_action;
};

class RateLimitPolicy {
  public:
    RateLimitPolicy(std::uint32_t max_concurrent = 10, std::uint32_t rate_limit_per_second = 100)
        : m_max_concurrent{max_concurrent}, m_rate_limit_per_second{rate_limit_per_second} {}

    void set_max_concurrent(std::uint32_t max_concurrent) noexcept { m_max_concurrent = max_concurrent; }
    void set_rate_limit_per_second(std::uint32_t rps) noexcept { m_rate_limit_per_second = rps; }

    [[nodiscard]] std::uint32_t get_max_concurrent() const noexcept { return m_max_concurrent; }
    [[nodiscard]] std::uint32_t get_rate_limit_per_second() const noexcept { return m_rate_limit_per_second; }

    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_max_concurrent == 0)        return std::unexpected{"max_concurrent must be at least 1"};
        if (m_rate_limit_per_second == 0) return std::unexpected{"rate_limit_per_second must be at least 1"};
        return {};
    }

  private:
    std::uint32_t m_max_concurrent;
    std::uint32_t m_rate_limit_per_second;
};

} // namespace model

template<> struct ser::Serializable<model::RetryPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"max_attempts",
                &model::RetryPolicy::get_max_attempts,
                &model::RetryPolicy::set_max_attempts>(),
            ser::field<"backoff",
                &model::RetryPolicy::get_backoff,
                &model::RetryPolicy::set_backoff>(),
            ser::field<"interval_ms",
                &model::RetryPolicy::get_interval_ms,
                &model::RetryPolicy::set_interval_ms>(),
        };
    }
};

template<> struct ser::Serializable<model::TimeoutPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"timeout_ms",
                &model::TimeoutPolicy::get_timeout_ms,
                &model::TimeoutPolicy::set_timeout_ms>(),
            ser::field<"action",
                &model::TimeoutPolicy::get_action,
                &model::TimeoutPolicy::set_action>(),
        };
    }
};

template<> struct ser::Serializable<model::RateLimitPolicy> {
    static constexpr auto fields() {
        return std::tuple{
            ser::field<"max_concurrent",
                &model::RateLimitPolicy::get_max_concurrent,
                &model::RateLimitPolicy::set_max_concurrent>(),
            ser::field<"rate_limit_per_second",
                &model::RateLimitPolicy::get_rate_limit_per_second,
                &model::RateLimitPolicy::set_rate_limit_per_second>(),
        };
    }
};
