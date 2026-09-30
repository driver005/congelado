// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"

export module cc_ice_extern_grappler_sonic:configs;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGrapplerConfigsOps : public ice::sonic::Runtime<TFGrapplerConfigsOps, TFGrapplerConfigsOps>
{
public:
    explicit TFGrapplerConfigsOps(TFGrapplerConfigsOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "grappler";

    void get_optimization_level(TFGrapplerOptimizationLevel* out_level) noexcept
    {
        m_ops->get_optimization_level(get_handle(), out_level);
    }

    void set_optimization_level(TFGrapplerOptimizationLevel level) noexcept
    {
        m_ops->set_optimization_level(get_handle(), level);
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_optimizer_configs(TFGrapplerOptimizerConfigs* out_configs) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_optimizer_configs(get_handle(), out_configs, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_optimizer_configs(const TFGrapplerOptimizerConfigs* in_configs) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_optimizer_configs(get_handle(), in_configs, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
