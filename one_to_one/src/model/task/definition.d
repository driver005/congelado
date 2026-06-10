module model.task.definition;

@nogc nothrow:

import model.task.status    : TaskType;
import model.common.policies : RetryPolicy, TimeoutPolicy, RateLimitPolicy;
import util.result           : Result;
import util.optional         : Optional;

// PORT-NOTE: C++ used std::vector<std::string> for input/output keys.
// D uses fixed-size arrays with count to avoid GC.
// Max 32 keys per direction — add IMPROVEMENTS entry if more are needed.

class TaskDef {
  public:
    void add_input_key(const(char)[] key) nothrow {
        // PORT-NOTE: C++ used std::vector::push_back; D uses fixed buffer[32].
        assert(m_input_keys_count < 32);
        m_input_keys_buf[m_input_keys_count++] = key;
    }
    void add_output_key(const(char)[] key) nothrow {
        assert(m_output_keys_count < 32);
        m_output_keys_buf[m_output_keys_count++] = key;
    }

    void set_name(const(char)[] name)              nothrow { m_name        = name;        }
    void set_type(TaskType type)                   nothrow { m_type        = type;        }
    void set_worker_type(const(char)[] type_)      nothrow { m_worker_type = type_;       }
    void set_input_keys(const(char)[][] input)     nothrow {
        // PORT-NOTE: C++ moved std::vector; D slices the caller's array (borrowed).
        size_t n = input.length < 32 ? input.length : 32;
        m_input_keys_buf[0 .. n] = input[0 .. n];
        m_input_keys_count = n;
    }
    void set_output_keys(const(char)[][] output)   nothrow {
        size_t n = output.length < 32 ? output.length : 32;
        m_output_keys_buf[0 .. n] = output[0 .. n];
        m_output_keys_count = n;
    }
    void set_retry(RetryPolicy retry)              nothrow { m_retry       = retry;       }
    void set_timeout(TimeoutPolicy timeout_)       nothrow { m_timeout     = timeout_;    }
    void set_rate_limit(Optional!RateLimitPolicy rate_limit) nothrow {
        m_rate_limit = rate_limit;
    }

    const(char)[] get_name()        const nothrow { return m_name;        }
    TaskType      get_type()        const nothrow { return m_type;        }
    const(char)[] get_worker_type() const nothrow { return m_worker_type; }
    const(char)[][] get_input_keys()  const nothrow {
        return cast(const(char)[][]) m_input_keys_buf[0 .. m_input_keys_count];
    }
    const(char)[][] get_output_keys() const nothrow {
        return cast(const(char)[][]) m_output_keys_buf[0 .. m_output_keys_count];
    }
    const(RetryPolicy)              get_retry()      const nothrow { return m_retry;      }
    const(TimeoutPolicy)            get_timeout()    const nothrow { return m_timeout;    }
    const(Optional!RateLimitPolicy) get_rate_limit() const nothrow { return m_rate_limit; }

    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_name.length == 0)
            return Result!(bool, const(char)[]).err("TaskDef name must not be empty");
        if (m_type == TaskType.SIMPLE && m_worker_type.length == 0)
            return Result!(bool, const(char)[]).err(
                "TaskDef worker_type must not be empty for SIMPLE tasks");
        {
            auto r = m_retry.validate();
            if (!r.is_ok) return r;
        }
        {
            auto r = m_timeout.validate();
            if (!r.is_ok) return r;
        }
        if (m_rate_limit.has_value) {
            auto r = m_rate_limit.value.validate();
            if (!r.is_ok) return r;
        }
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    const(char)[] m_name;
    TaskType      m_type = TaskType.SIMPLE;
    const(char)[] m_worker_type;
    // PORT-NOTE: C++ std::vector replaced by fixed-size buffer+count for @nogc.
    const(char)[][32] m_input_keys_buf;
    size_t            m_input_keys_count;
    const(char)[][32] m_output_keys_buf;
    size_t            m_output_keys_count;
    RetryPolicy       m_retry;
    TimeoutPolicy     m_timeout;
    Optional!RateLimitPolicy m_rate_limit;
}
