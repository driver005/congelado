export module model:policies;

import std;
import serde;

export namespace model {

enum class RetryBackoff : std::uint8_t { FIXED, EXPONENTIAL };

class RetryPolicy {
  public:
    /**
     * @brief Builds a retry policy — defaults to 3 attempts, fixed backoff, 1s between tries.
     * That's a sane baseline, bet, tune it per task if it's not enough.
     * @param max_attempts how many times a task gets retried before it's given up on.
     * @param backoff whether the wait between attempts stays flat or grows exponentially.
     * @param interval_ms base wait time between attempts, in milliseconds.
     */
    RetryPolicy(std::uint32_t max_attempts = 3, RetryBackoff backoff = RetryBackoff::FIXED,
                std::uint32_t interval_ms = 1000)
        : m_max_attempts{max_attempts}, m_backoff{backoff}, m_interval_ms{interval_ms} {}

    /// @brief Sets the max retry attempts.
    /// @param max_attempts how many attempts a task gets before it's marked failed.
    void set_max_attempts(std::uint32_t max_attempts) noexcept { m_max_attempts = max_attempts; }
    /// @brief Sets the backoff strategy used between retries.
    /// @param backoff FIXED for a flat interval, EXPONENTIAL to grow it each attempt.
    void set_backoff(RetryBackoff backoff) noexcept { m_backoff = backoff; }
    /// @brief Sets the base interval between retry attempts.
    /// @param interval_ms wait time in milliseconds.
    void set_interval_ms(std::uint32_t interval_ms) noexcept { m_interval_ms = interval_ms; }

    /// @brief Gets the configured max retry attempts.
    /// @return how many attempts a task gets.
    [[nodiscard]] std::uint32_t get_max_attempts() const noexcept { return m_max_attempts; }
    /// @brief Gets the configured backoff strategy.
    /// @return FIXED or EXPONENTIAL.
    [[nodiscard]] RetryBackoff get_backoff() const noexcept { return m_backoff; }
    /// @brief Gets the configured base retry interval.
    /// @return the interval in milliseconds.
    [[nodiscard]] std::uint32_t get_interval_ms() const noexcept { return m_interval_ms; }

    /**
     * @brief Checks that this policy is actually usable — no zero attempts, no zero interval.
     * @return an empty expected if the policy's good to go, otherwise an unexpected carrying a
     * message naming the busted field.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // Zero attempts means the task never gets a first try — check that before anything else.
        if (m_max_attempts == 0) {
            return std::unexpected{"max_attempts must be at least 1"};
        }
        // Same deal for the interval — a zero wait between retries isn't really backoff at all, bet.
        if (m_interval_ms == 0) {
            return std::unexpected{"interval_ms must be greater than 0"};
        }
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
    /**
     * @brief Builds a timeout policy — defaults to 30s, failing the whole workflow, no cap, if
     * it's hit.
     * @param timeout_ms how long to wait before this policy kicks in, in milliseconds.
     * @param action what happens once the timeout fires.
     */
    TimeoutPolicy(std::uint32_t timeout_ms = 30000,
                  TimeoutAction action = TimeoutAction::FAIL_WORKFLOW)
        : m_timeout_ms{timeout_ms}, m_action{action} {}

    /// @brief Sets the timeout duration.
    /// @param timeout_ms how long to wait before timing out, in milliseconds.
    void set_timeout_ms(std::uint32_t timeout_ms) noexcept { m_timeout_ms = timeout_ms; }
    /// @brief Sets what happens when the timeout fires.
    /// @param action RETRY, FAIL_WORKFLOW, or ALERT_ONLY.
    void set_action(TimeoutAction action) noexcept { m_action = action; }

    /// @brief Gets the configured timeout duration.
    /// @return the timeout in milliseconds.
    [[nodiscard]] std::uint32_t get_timeout_ms() const noexcept { return m_timeout_ms; }
    /// @brief Gets the configured timeout action.
    /// @return RETRY, FAIL_WORKFLOW, or ALERT_ONLY.
    [[nodiscard]] TimeoutAction get_action() const noexcept { return m_action; }

    /**
     * @brief Checks that the timeout duration is actually usable — lowkey the simplest
     * validate() in this file.
     * @return an empty expected if timeout_ms is nonzero, otherwise an unexpected explaining
     * why.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        if (m_timeout_ms == 0) {
            return std::unexpected{"timeout_ms must be greater than 0"};
        }
        return {};
    }

  private:
    std::uint32_t m_timeout_ms;
    TimeoutAction m_action;
};

class RateLimitPolicy {
  public:
    /**
     * @brief Builds a rate-limit policy, motion straight out the gate — defaults to 10
     * concurrent slots, 100 req/s.
     * @param max_concurrent the max number of concurrent in-flight executions allowed.
     * @param rate_limit_per_second the max throughput allowed, in requests per second.
     */
    RateLimitPolicy(std::uint32_t max_concurrent = 10, std::uint32_t rate_limit_per_second = 100)
        : m_max_concurrent{max_concurrent}, m_rate_limit_per_second{rate_limit_per_second} {}

    /// @brief Sets the max concurrent in-flight executions allowed.
    /// @param max_concurrent the new concurrency cap.
    void set_max_concurrent(std::uint32_t max_concurrent) noexcept {
        m_max_concurrent = max_concurrent;
    }
    /// @brief Sets the max throughput allowed.
    /// @param rps the new rate limit, in requests per second.
    void set_rate_limit_per_second(std::uint32_t rps) noexcept { m_rate_limit_per_second = rps; }

    /// @brief Gets the configured max concurrent in-flight executions.
    /// @return the concurrency cap.
    [[nodiscard]] std::uint32_t get_max_concurrent() const noexcept { return m_max_concurrent; }
    /// @brief Gets the configured max throughput.
    /// @return the rate limit, in requests per second.
    [[nodiscard]] std::uint32_t get_rate_limit_per_second() const noexcept {
        return m_rate_limit_per_second;
    }

    /**
     * @brief Checks that both the concurrency cap and the rate limit are nonzero.
     * @return an empty expected if the policy's W, otherwise an unexpected naming which field
     * is zeroed out.
     */
    [[nodiscard]] std::expected<void, std::string> validate() const noexcept {
        // No concurrency slots means nothing can ever run, so that's checked first.
        if (m_max_concurrent == 0) {
            return std::unexpected{"max_concurrent must be at least 1"};
        }
        // Then make sure the throughput cap itself isn't zeroed out too.
        if (m_rate_limit_per_second == 0) {
            return std::unexpected{"rate_limit_per_second must be at least 1"};
        }
        return {};
    }

  private:
    std::uint32_t m_max_concurrent;
    std::uint32_t m_rate_limit_per_second;
};

} // namespace model

template <>
struct serde::Serializable<model::RetryPolicy> {
    /**
     * @brief Field-descriptor table wiring RetryPolicy's max_attempts/backoff/interval_ms to
     * their getters/setters, for serde (de)serialization.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"max_attempts", &model::RetryPolicy::get_max_attempts,
                       &model::RetryPolicy::set_max_attempts>{},
            serde::FieldDesc<"backoff", &model::RetryPolicy::get_backoff,
                       &model::RetryPolicy::set_backoff>{},
            serde::FieldDesc<"interval_ms", &model::RetryPolicy::get_interval_ms,
                       &model::RetryPolicy::set_interval_ms>{},
        };
    }
};

template <>
struct serde::Serializable<model::TimeoutPolicy> {
    /**
     * @brief Field-descriptor table wiring TimeoutPolicy's timeout_ms/action to their
     * getters/setters, for serde (de)serialization — bet, that's the whole table.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"timeout_ms", &model::TimeoutPolicy::get_timeout_ms,
                       &model::TimeoutPolicy::set_timeout_ms>{},
            serde::FieldDesc<"action", &model::TimeoutPolicy::get_action,
                       &model::TimeoutPolicy::set_action>{},
        };
    }
};

template <>
struct serde::Serializable<model::RateLimitPolicy> {
    /**
     * @brief Field-descriptor table wiring RateLimitPolicy's max_concurrent/
     * rate_limit_per_second to their getters/setters, for serde (de)serialization.
     * @return the tuple of FieldDesc entries serde uses for this type.
     */
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"max_concurrent", &model::RateLimitPolicy::get_max_concurrent,
                       &model::RateLimitPolicy::set_max_concurrent>{},
            serde::FieldDesc<"rate_limit_per_second", &model::RateLimitPolicy::get_rate_limit_per_second,
                       &model::RateLimitPolicy::set_rate_limit_per_second>{},
        };
    }
};
