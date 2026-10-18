// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"

export module cc_ice_extern_grappler_sonic:tri_state;

import std;

export namespace ice::sonic {

class TriState
{
public:
    enum Value : std::underlying_type_t<::TF_TriState>
    {
        k_default = ::TF_TriState_Default,
        k_off = ::TF_TriState_Off,
        k_on = ::TF_TriState_On
    };

    constexpr TriState() noexcept = default;

    constexpr TriState(::TF_TriState value) noexcept :
        m_value{value}
    {
    }

    constexpr TriState(Value value) noexcept :
        m_value{static_cast<::TF_TriState>(value)}
    {
    }

    constexpr ::TF_TriState get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TF_TriState() const noexcept
    {
        return m_value;
    }

private:
    ::TF_TriState m_value{};
};

} // namespace ice::sonic
