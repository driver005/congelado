export module cc_abi_gen_writer:helper_diff;

import std;

export namespace cc_abi_gen::helper {

class DiffResult
{
public:
    DiffResult(
        bool identical,
        const std::string&& unified_diff,
        std::chrono::milliseconds duration
    ) :
        m_identical{identical},
        m_unified_diff{std::move(unified_diff)},
        m_duration{duration}
    {
    }

    ~DiffResult() = default;
    DiffResult(const DiffResult&) = delete;
    DiffResult& operator=(const DiffResult&) = delete;
    DiffResult(DiffResult&&) = default;
    DiffResult& operator=(DiffResult&&) = default;

    DiffResult& add_identical(bool identical)
    {
        m_identical = identical;
        return *this;
    }

    DiffResult& add_unified_diff(const std::string&& unified_diff) noexcept
    {
        m_unified_diff = std::move(unified_diff);
        return *this;
    }

    DiffResult& add_duration(std::chrono::milliseconds&& duration) noexcept
    {
        m_duration = std::move(duration);
        return *this;
    }

    void set_identical(bool identical) noexcept
    {
        m_identical = identical;
    }

    void set_unified_diff(std::string&& unified_diff) noexcept
    {
        m_unified_diff = std::move(unified_diff);
    }

    void set_duration(std::chrono::milliseconds&& duration) noexcept
    {
        m_duration = std::move(duration);
    }

    bool get_identical() const noexcept
    {
        return m_identical;
    }

    const std::string& get_unified_diff() const noexcept
    {
        return m_unified_diff;
    }

    const std::chrono::milliseconds& get_duration() const noexcept
    {
        return m_duration;
    }

private:
    bool m_identical;
    std::string m_unified_diff;
    std::chrono::milliseconds m_duration;
};

} // namespace cc_abi_gen::helper
