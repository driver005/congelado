module;

#include "include/c/intern/bitset.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module aten_xpu_intern:bit_set;

import std;
import cc_ice_intern_builder;
import cc_ice_intern_sonic;
import :failure;

export namespace aten_xpu {

// A fixed size bit set; the ABI has no size slot, so the size comes from the constructor.
class BitSet : public ice::builder::BitSet
{
public:
    BitSet(
        const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        std::size_t size
    ) noexcept :
        ice::builder::BitSet{status_ops},
        m_failure{string_ops},
        m_bits(size, false)
    {
    }

    ~BitSet() override = default;
    BitSet(const BitSet&) = delete;
    BitSet& operator=(const BitSet&) = delete;
    BitSet(BitSet&&) = delete;
    BitSet& operator=(BitSet&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    // Out of range indices are ignored here because set and clear have no status.
    void set(std::size_t index) noexcept override
    {
        if (index < m_bits.size()) {
            m_bits[index] = true;
        }
    }

    void clear(std::size_t index) noexcept override
    {
        if (index < m_bits.size()) {
            m_bits[index] = false;
        }
    }

    void test(
        std::size_t index,
        int& out_result,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (index >= m_bits.size()) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "bit out of range");
            return;
        }

        out_result = m_bits[index] ? 1 : 0;
    }

    void flip(std::size_t index, const ice::sonic::Status& out_status) noexcept override
    {
        if (index >= m_bits.size()) {
            m_failure.fail(out_status, ice::sonic::Code::k_out_of_range, "bit out of range");
            return;
        }

        m_bits[index] = !m_bits[index];
    }

    void count(std::size_t& out_count) noexcept override
    {
        out_count = static_cast<std::size_t>(std::ranges::count(m_bits, true));
    }

    void size(std::size_t& out_size) noexcept override
    {
        out_size = m_bits.size();
    }

private:
    Failure m_failure;
    std::vector<bool> m_bits;
};

} // namespace aten_xpu
