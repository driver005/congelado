module model.common.policies;

@nogc nothrow:

import util.result : Result;

// ─── RetryBackoff ──────────────────────────────────────────────────────────────

enum RetryBackoff : ubyte {
    FIXED,
    EXPONENTIAL,
}

// ─── RetryPolicy ──────────────────────────────────────────────────────────────

class RetryPolicy {
  public:
    this(uint max_attempts = 3, RetryBackoff backoff = RetryBackoff.FIXED,
         uint interval_ms = 1000) nothrow {
        m_max_attempts = max_attempts;
        m_backoff      = backoff;
        m_interval_ms  = interval_ms;
    }

    void set_max_attempts(uint max_attempts) nothrow { m_max_attempts = max_attempts; }
    void set_backoff(RetryBackoff backoff)   nothrow { m_backoff      = backoff;      }
    void set_interval_ms(uint interval_ms)  nothrow { m_interval_ms  = interval_ms;  }

    uint         get_max_attempts() const nothrow { return m_max_attempts; }
    RetryBackoff get_backoff()      const nothrow { return m_backoff;      }
    uint         get_interval_ms()  const nothrow { return m_interval_ms;  }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_max_attempts == 0)
            return Result!(bool, const(char)[]).err("max_attempts must be at least 1");
        if (m_interval_ms == 0)
            return Result!(bool, const(char)[]).err("interval_ms must be greater than 0");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    uint         m_max_attempts;
    RetryBackoff m_backoff;
    uint         m_interval_ms;
}

// ─── TimeoutAction ────────────────────────────────────────────────────────────

enum TimeoutAction : ubyte {
    RETRY,
    FAIL_WORKFLOW,
    ALERT_ONLY,
}

// ─── TimeoutPolicy ────────────────────────────────────────────────────────────

class TimeoutPolicy {
  public:
    this(uint timeout_ms = 30000, TimeoutAction action = TimeoutAction.FAIL_WORKFLOW) nothrow {
        m_timeout_ms = timeout_ms;
        m_action     = action;
    }

    void set_timeout_ms(uint timeout_ms) nothrow { m_timeout_ms = timeout_ms; }
    void set_action(TimeoutAction action) nothrow { m_action    = action;     }

    uint          get_timeout_ms() const nothrow { return m_timeout_ms; }
    TimeoutAction get_action()     const nothrow { return m_action;     }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_timeout_ms == 0)
            return Result!(bool, const(char)[]).err("timeout_ms must be greater than 0");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    uint          m_timeout_ms;
    TimeoutAction m_action;
}

// ─── RateLimitPolicy ──────────────────────────────────────────────────────────

class RateLimitPolicy {
  public:
    this(uint max_concurrent = 10, uint rate_limit_per_second = 100) nothrow {
        m_max_concurrent          = max_concurrent;
        m_rate_limit_per_second   = rate_limit_per_second;
    }

    void set_max_concurrent(uint max_concurrent) nothrow         { m_max_concurrent        = max_concurrent;        }
    void set_rate_limit_per_second(uint rps)     nothrow         { m_rate_limit_per_second = rps;                   }

    uint get_max_concurrent()        const nothrow { return m_max_concurrent;        }
    uint get_rate_limit_per_second() const nothrow { return m_rate_limit_per_second; }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_max_concurrent == 0)
            return Result!(bool, const(char)[]).err("max_concurrent must be at least 1");
        if (m_rate_limit_per_second == 0)
            return Result!(bool, const(char)[]).err("rate_limit_per_second must be at least 1");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    uint m_max_concurrent;
    uint m_rate_limit_per_second;
}
