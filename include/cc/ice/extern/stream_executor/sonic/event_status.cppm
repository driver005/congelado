// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/event.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/event.h"

export module cc_ice_extern_stream_executor_sonic:event_status;

import std;

export namespace ice::sonic {

class EventStatus
{
public:
    enum Value : std::underlying_type_t<::TF_EventStatus>
    {
        k_unknown = ::TF_EVENT_UNKNOWN,
        k_error = ::TF_EVENT_ERROR,
        k_pending = ::TF_EVENT_PENDING,
        k_complete = ::TF_EVENT_COMPLETE
    };

    constexpr EventStatus() noexcept = default;

    constexpr EventStatus(::TF_EventStatus value) noexcept :
        m_value{value}
    {
    }

    constexpr EventStatus(Value value) noexcept :
        m_value{static_cast<::TF_EventStatus>(value)}
    {
    }

    constexpr ::TF_EventStatus get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_EventStatus() const noexcept
    {
        return m_value;
    }

private:
    ::TF_EventStatus m_value{};
};

} // namespace ice::sonic
