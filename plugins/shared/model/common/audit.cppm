module;

export module model:audit;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace model {

class AuditRecord
{
public:
    /// @brief Default ctor — bet, nothing fancy: timestamps sit at the epoch and version starts
    /// at 0 until someone actually stamps this record.
    AuditRecord() = default;

    /// @brief Sets the record's creation timestamp.
    /// @note Param is named `updated_at` even though this is the created_at setter — just how
    /// it's written, don't let it throw you off.
    /// @param updated_at the creation time to stamp on this record.
    void set_created_at(std::chrono::system_clock::time_point updated_at) noexcept
    {
        m_created_at = updated_at;
    }

    /// @brief Sets the record's last-updated timestamp.
    /// @param updated_at the update time to stamp on this record.
    void set_updated_at(std::chrono::system_clock::time_point updated_at) noexcept
    {
        m_updated_at = updated_at;
    }

    /// @brief Sets the optimistic-concurrency version counter.
    /// @warning This just overwrites the counter — it's on you (or the persistence layer) to
    /// bump it correctly. Set it wrong and you're cooked: two writers both thinking they're on
    /// version 3 is exactly how lost updates happen.
    /// @param version the new version number.
    void set_version(std::uint32_t version) noexcept
    {
        m_version = version;
    }

    /// @brief Gets the record's creation timestamp.
    /// @return the time this record was created.
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept
    {
        return m_created_at;
    }

    /// @brief Gets the record's last-updated timestamp.
    /// @return the time this record was last touched.
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    /// @brief Gets the optimistic-concurrency version counter.
    /// @return the current version number, no cap.
    [[nodiscard]] std::uint32_t get_version() const noexcept
    {
        return m_version;
    }

private:
    std::chrono::system_clock::time_point m_created_at;
    std::chrono::system_clock::time_point m_updated_at;
    std::uint32_t m_version{0};
};

} // namespace model

#ifdef CONGELADO_TEST
namespace model::tests {
using namespace boost::ut;

suite<"AuditRecord"> audit_record_suite = [] {
    "defaults to epoch timestamps and version 0"_test = [] {
        AuditRecord record;

        expect(record.get_created_at() == std::chrono::system_clock::time_point{});
        expect(record.get_updated_at() == std::chrono::system_clock::time_point{});
        expect(record.get_version() == 0);
    };
    "setters round-trip through their getters"_test = [] {
        AuditRecord record;
        auto now = std::chrono::system_clock::now();

        record.set_created_at(now);
        record.set_updated_at(now);
        record.set_version(3);

        expect(record.get_created_at() == now);
        expect(record.get_updated_at() == now);
        expect(record.get_version() == 3);
    };
};

} // namespace model::tests
#endif
