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

    bool get_identical() const
    {
        return m_identical;
    }

    const std::string& get_unified_diff() const
    {
        return m_unified_diff;
    }

    std::chrono::milliseconds get_duration() const
    {
        return m_duration;
    }

private:
    bool m_identical;
    std::string m_unified_diff;
    std::chrono::milliseconds m_duration;
};

} // namespace cc_abi_gen::helper
