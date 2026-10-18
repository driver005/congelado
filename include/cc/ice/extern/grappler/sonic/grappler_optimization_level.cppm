// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"

export module cc_ice_extern_grappler_sonic:grappler_optimization_level;

import std;

export namespace ice::sonic {

class GrapplerOptimizationLevel
{
public:
    enum Value : std::underlying_type_t<::TFGrapplerOptimizationLevel>
    {
        k_l0 = ::TF_GRAPPLER_OPT_L0,
        k_l1 = ::TF_GRAPPLER_OPT_L1,
        k_l2 = ::TF_GRAPPLER_OPT_L2,
        k_l3 = ::TF_GRAPPLER_OPT_L3
    };

    constexpr GrapplerOptimizationLevel() noexcept = default;

    constexpr GrapplerOptimizationLevel(::TFGrapplerOptimizationLevel value) noexcept :
        m_value{value}
    {
    }

    constexpr GrapplerOptimizationLevel(Value value) noexcept :
        m_value{static_cast<::TFGrapplerOptimizationLevel>(value)}
    {
    }

    constexpr ::TFGrapplerOptimizationLevel get_value() const noexcept
    {
        return m_value;
    }

    constexpr operator ::TFGrapplerOptimizationLevel() const noexcept
    {
        return m_value;
    }

private:
    ::TFGrapplerOptimizationLevel m_value{};
};

} // namespace ice::sonic
