// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/allocator.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/allocator.h"

export module cc_ice_extern_stream_executor_sonic:allocator_option;

import std;

export namespace ice::sonic {

class AllocatorOption
{
public:
    enum Value : std::underlying_type_t<::TF_AllocatorOption>
    {
        k_expandable_segments = ::TF_ALLOCATOR_OPTION_EXPANDABLE_SEGMENTS,
        k_no_split = ::TF_ALLOCATOR_OPTION_NO_SPLIT,
        k_use_on_oom = ::TF_ALLOCATOR_OPTION_USE_ON_OOM,
        k_max_split_size = ::TF_ALLOCATOR_OPTION_MAX_SPLIT_SIZE,
        k_garbage_collection_threshold = ::TF_ALLOCATOR_OPTION_GARBAGE_COLLECTION_THRESHOLD,
        k_round_up_power2_divisions = ::TF_ALLOCATOR_OPTION_ROUND_UP_POWER2_DIVISIONS
    };

    constexpr AllocatorOption() noexcept = default;

    constexpr AllocatorOption(::TF_AllocatorOption value) noexcept :
        m_value{value}
    {
    }

    constexpr AllocatorOption(Value value) noexcept :
        m_value{static_cast<::TF_AllocatorOption>(value)}
    {
    }

    constexpr ::TF_AllocatorOption get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_AllocatorOption() const noexcept
    {
        return m_value;
    }

private:
    ::TF_AllocatorOption m_value{};
};

} // namespace ice::sonic
