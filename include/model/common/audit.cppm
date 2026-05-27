module;

export module model:audit;

import std;

export namespace model {

class AuditRecord {
  public:
    AuditRecord() = default;

    void set_created_at(std::chrono::system_clock::time_point updated_at) noexcept { m_created_at = updated_at; }
    void set_updated_at(std::chrono::system_clock::time_point updated_at) noexcept { m_updated_at = updated_at; }
    void set_version(std::uint32_t version) noexcept { m_version = version; }

    [[nodiscard]] const std::chrono::system_clock::time_point &get_created_at() const noexcept { return m_created_at; }
    [[nodiscard]] const std::chrono::system_clock::time_point &get_updated_at() const noexcept { return m_updated_at; }
    [[nodiscard]] std::uint32_t get_version() const noexcept { return m_version; }

  private:
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
    std::uint32_t m_version{0};
};

} // namespace model
