export module model:policies;

import std;

export namespace model {

enum class RetryBackoff : std::uint8_t {
    FIXED,
    EXPONENTIAL,
};

struct RetryPolicy {
    std::uint32_t max_attempts{3};
    RetryBackoff  backoff{RetryBackoff::FIXED};
    std::uint32_t interval_ms{1000};
};

enum class TimeoutAction : std::uint8_t {
    RETRY,
    FAIL_WORKFLOW,
    ALERT_ONLY,
};

struct TimeoutPolicy {
    std::uint32_t timeout_ms{30000};
    TimeoutAction action{TimeoutAction::FAIL_WORKFLOW};
};

struct RateLimitPolicy {
    std::uint32_t max_concurrent{10};
    std::uint32_t rate_limit_per_second{100};
};

} // namespace model
