// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/stream.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/stream.h"

export module cc_ice_extern_stream_executor_sonic:capture_status;

import std;

export namespace ice::sonic {

class CaptureStatus
{
public:
    enum Value : std::underlying_type_t<::TF_CaptureStatus>
    {
        k_none = ::TF_CAPTURE_STATUS_NONE,
        k_active = ::TF_CAPTURE_STATUS_ACTIVE
    };

    constexpr CaptureStatus() noexcept = default;

    constexpr CaptureStatus(::TF_CaptureStatus value) noexcept :
        m_value{value}
    {
    }

    constexpr CaptureStatus(Value value) noexcept :
        m_value{static_cast<::TF_CaptureStatus>(value)}
    {
    }

    constexpr ::TF_CaptureStatus get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_CaptureStatus() const noexcept
    {
        return m_value;
    }

private:
    ::TF_CaptureStatus m_value{};
};

} // namespace ice::sonic
