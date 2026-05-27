export module model:policies;

import std;

export namespace model {

enum class RetryBackoff : std::uint8_t {
    FIXED,
    EXPONENTIAL,
};

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

  private:
    std::uint32_t m_max_attempts;
    RetryBackoff m_backoff;
    std::uint32_t m_interval_ms;
};

enum class TimeoutAction : std::uint8_t {
    RETRY,
    FAIL_WORKFLOW,
    ALERT_ONLY,
};

class TimeoutPolicy {
  public:
    TimeoutPolicy(std::uint32_t timeout_ms = 30000, TimeoutAction action = TimeoutAction::FAIL_WORKFLOW)
        : m_timeout_ms{timeout_ms}, m_action{action} {}

    void set_timeout_ms(std::uint32_t timeout_ms) noexcept { m_timeout_ms = timeout_ms; }
    void set_action(TimeoutAction action) noexcept { m_action = action; }

    [[nodiscard]] std::uint32_t get_timeout_ms() const noexcept { return m_timeout_ms; }
    [[nodiscard]] TimeoutAction get_action() const noexcept { return m_action; }

  private:
    std::uint32_t m_timeout_ms;
    TimeoutAction m_action;
};

class RateLimitPolicy {
  public:
    RateLimitPolicy(std::uint32_t max_concurrent = 10, std::uint32_t rate_limit_per_second = 100)
        : m_max_concurrent{max_concurrent}, m_rate_limit_per_second{rate_limit_per_second} {}

    void set_max_concurrent(std::uint32_t max_concurrent) noexcept { m_max_concurrent = max_concurrent; }
    void set_rate_limit_per_second(std::uint32_t rate_limit_per_second) noexcept {
        m_rate_limit_per_second = rate_limit_per_second;
    }

    [[nodiscard]] std::uint32_t get_rate_limit_per_second() const noexcept { return m_rate_limit_per_second; }
    [[nodiscard]] std::uint32_t get_max_concurrent() const noexcept { return m_max_concurrent; }

  private:
    std::uint32_t m_max_concurrent;
    std::uint32_t m_rate_limit_per_second;
};

} // namespace model
