// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/mem_pool.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/mem_pool.h"

export module cc_ice_extern_stream_executor_sonic:pool_id;

import std;

export namespace ice::sonic {

class PoolId
{
public:
    PoolId() noexcept {}

    explicit PoolId(const ::TF_PoolId& value) noexcept :
        m_value{value}
    {
    }

    PoolId(std::int64_t first, std::int64_t second) noexcept
    {
        m_value.first = first;
        m_value.second = second;
    }

    std::int64_t getFirst() const noexcept
    {
        return m_value.first;
    }

    void setFirst(std::int64_t value) noexcept
    {
        m_value.first = value;
    }

    std::int64_t getSecond() const noexcept
    {
        return m_value.second;
    }

    void setSecond(std::int64_t value) noexcept
    {
        m_value.second = value;
    }

    ::TF_PoolId* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_PoolId* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_PoolId m_value{};
};

} // namespace ice::sonic
