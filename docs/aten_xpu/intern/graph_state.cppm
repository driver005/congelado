module;

export module aten_xpu_intern:graph_state;

import std;

export namespace aten_xpu {

class SyclGraphState
{
public:
    void setDebugMode(bool debug_mode) noexcept
    {
        m_debug_mode = debug_mode;
    }

    bool getDebugMode() const noexcept
    {
        return m_debug_mode;
    }

    std::uint64_t next_pool_sequence() noexcept
    {
        return ++m_pool_sequence;
    }

private:
    bool m_debug_mode{false};
    std::uint64_t m_pool_sequence{0};
};

} // namespace aten_xpu
