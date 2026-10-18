// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/random_generator/random_generator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/random_generator/random_generator.h"

export module cc_ice_extern_random_generator_sonic:philox_state;

import std;

export namespace ice::sonic {

class PhiloxState
{
public:
    PhiloxState() noexcept
    {
        m_value.struct_size = sizeof(::TF_PhiloxState);
    }

    explicit PhiloxState(const ::TF_PhiloxState& value) noexcept :
        m_value{value}
    {
    }

    PhiloxState(
        std::uint64_t seed,
        std::uint64_t offset,
        const std::int64_t* seed_ptr,
        const std::int64_t* offset_ptr,
        std::uint32_t offset_intragraph,
        _Bool captured
    ) noexcept
    {
        m_value.struct_size = sizeof(::TF_PhiloxState);
        m_value.seed = seed;
        m_value.offset = offset;
        m_value.seed_ptr = seed_ptr;
        m_value.offset_ptr = offset_ptr;
        m_value.offset_intragraph = offset_intragraph;
        m_value.captured = captured;
    }

    std::uint64_t getSeed() const noexcept
    {
        return m_value.seed;
    }

    void setSeed(std::uint64_t value) noexcept
    {
        m_value.seed = value;
    }

    std::uint64_t getOffset() const noexcept
    {
        return m_value.offset;
    }

    void setOffset(std::uint64_t value) noexcept
    {
        m_value.offset = value;
    }

    const std::int64_t* getSeedPtr() const noexcept
    {
        return m_value.seed_ptr;
    }

    void setSeedPtr(const std::int64_t* value) noexcept
    {
        m_value.seed_ptr = value;
    }

    const std::int64_t* getOffsetPtr() const noexcept
    {
        return m_value.offset_ptr;
    }

    void setOffsetPtr(const std::int64_t* value) noexcept
    {
        m_value.offset_ptr = value;
    }

    std::uint32_t getOffsetIntragraph() const noexcept
    {
        return m_value.offset_intragraph;
    }

    void setOffsetIntragraph(std::uint32_t value) noexcept
    {
        m_value.offset_intragraph = value;
    }

    _Bool getCaptured() const noexcept
    {
        return m_value.captured;
    }

    void setCaptured(_Bool value) noexcept
    {
        m_value.captured = value;
    }

    ::TF_PhiloxState* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_PhiloxState* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_PhiloxState m_value{};
};

} // namespace ice::sonic
