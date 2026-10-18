// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/types.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/types.h"

export module cc_ice_extern_stream_executor_sonic:memory_space;

import std;

export namespace ice::sonic {

class MemorySpace
{
public:
    enum Value : std::underlying_type_t<::TF_MemorySpace>
    {
        k_device = ::TF_MEMORY_SPACE_DEVICE,
        k_host_pinned = ::TF_MEMORY_SPACE_HOST_PINNED,
        k_unified = ::TF_MEMORY_SPACE_UNIFIED
    };

    constexpr MemorySpace() noexcept = default;

    constexpr MemorySpace(::TF_MemorySpace value) noexcept :
        m_value{value}
    {
    }

    constexpr MemorySpace(Value value) noexcept :
        m_value{static_cast<::TF_MemorySpace>(value)}
    {
    }

    constexpr ::TF_MemorySpace get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_MemorySpace() const noexcept
    {
        return m_value;
    }

private:
    ::TF_MemorySpace m_value{};
};

} // namespace ice::sonic
