// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/executor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/executor.h"

export module cc_ice_extern_stream_executor_sonic:stream_options;

import std;

export namespace ice::sonic {

class StreamOptions
{
public:
    StreamOptions() noexcept
    {
        m_value.struct_size = sizeof(::TF_StreamOptions);
    }

    explicit StreamOptions(const ::TF_StreamOptions& value) noexcept :
        m_value{value}
    {
    }

    explicit StreamOptions(std::int32_t priority) noexcept
    {
        m_value.struct_size = sizeof(::TF_StreamOptions);
        m_value.priority = priority;
    }

    std::int32_t getPriority() const noexcept
    {
        return m_value.priority;
    }

    void setPriority(std::int32_t value) noexcept
    {
        m_value.priority = value;
    }

    ::TF_StreamOptions* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_StreamOptions* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_StreamOptions m_value{};
};

} // namespace ice::sonic
