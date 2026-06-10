module model.common.timestamps;

@nogc nothrow:

import util.result : Result;

// PORT-NOTE: C++ used std::chrono::system_clock::time_point.
// D port uses long (Unix epoch milliseconds) with PORT-NOTE on each field.
// Validation logic is preserved exactly.

import util.optional : Optional;

class ExecutionTimings {
  public:
    void set_scheduled_at(Optional!long tp) nothrow { m_scheduled_at = tp; }
    void set_started_at(Optional!long tp) nothrow   { m_started_at   = tp; }
    void set_completed_at(Optional!long tp) nothrow { m_completed_at = tp; }

    const(Optional!long) get_scheduled_at() const nothrow { return m_scheduled_at; }
    const(Optional!long) get_started_at()   const nothrow { return m_started_at;   }
    const(Optional!long) get_completed_at() const nothrow { return m_completed_at; }

    // PORT-NOTE: C++ returned std::expected<void, std::string>.
    // D port returns Result!(bool, const(char)[]) where bool=true means valid.
    Result!(bool, const(char)[]) validate() const nothrow {
        if (m_scheduled_at.has_value && m_started_at.has_value
                && m_started_at.value < m_scheduled_at.value)
            return Result!(bool, const(char)[]).err("started_at must not be before scheduled_at");
        if (m_started_at.has_value && m_completed_at.has_value
                && m_completed_at.value < m_started_at.value)
            return Result!(bool, const(char)[]).err("completed_at must not be before started_at");
        return Result!(bool, const(char)[]).ok(true);
    }

  private:
    // PORT-NOTE: C++ used std::optional<std::chrono::system_clock::time_point>.
    // D uses Optional!long (Unix epoch ms).
    Optional!long m_scheduled_at;
    Optional!long m_started_at;
    Optional!long m_completed_at;
}
