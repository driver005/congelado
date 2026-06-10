module model.common.audit;

@nogc nothrow:

// PORT-NOTE: C++ used std::chrono::system_clock::time_point.
// D port uses long (Unix epoch milliseconds) for each timestamp field.

class AuditRecord {
  public:
    void set_created_at(long updated_at) nothrow { m_created_at = updated_at; }
    void set_updated_at(long updated_at) nothrow { m_updated_at = updated_at; }
    void set_version(uint version_) nothrow      { m_version    = version_;   }

    long get_created_at() const nothrow { return m_created_at; }
    long get_updated_at() const nothrow { return m_updated_at; }
    uint get_version()    const nothrow { return m_version;    }

  private:
    // PORT-NOTE: C++ used std::chrono::system_clock::time_point; D uses long (Unix epoch ms).
    long m_created_at;
    long m_updated_at;
    uint m_version = 0;
}
