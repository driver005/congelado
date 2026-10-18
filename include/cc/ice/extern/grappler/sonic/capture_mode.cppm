// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/device_graph.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/device_graph.h"

export module cc_ice_extern_grappler_sonic:capture_mode;

import std;

export namespace ice::sonic {

class CaptureMode
{
public:
    enum Value : std::underlying_type_t<::TF_CaptureMode>
    {
        k_global = ::TF_CAPTURE_MODE_GLOBAL,
        k_thread_local = ::TF_CAPTURE_MODE_THREAD_LOCAL,
        k_relaxed = ::TF_CAPTURE_MODE_RELAXED
    };

    constexpr CaptureMode() noexcept = default;

    constexpr CaptureMode(::TF_CaptureMode value) noexcept :
        m_value{value}
    {
    }

    constexpr CaptureMode(Value value) noexcept :
        m_value{static_cast<::TF_CaptureMode>(value)}
    {
    }

    constexpr ::TF_CaptureMode get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_CaptureMode() const noexcept
    {
        return m_value;
    }

private:
    ::TF_CaptureMode m_value{};
};

} // namespace ice::sonic
