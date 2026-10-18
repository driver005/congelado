// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_grappler_sonic:configs;

import std;
import :grappler_optimization_level;
import :grappler_optimizer_configs;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class GrapplerConfigs : public ice::sonic::Runtime<::TFGrapplerConfigsOps, ::TFGrapplerConfigs>
{
public:
    GrapplerConfigs(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    GrapplerConfigs(
        const ice::sonic::Registration& registry,
        const ::TFGrapplerConfigs& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit GrapplerConfigs(const ::TFGrapplerConfigsOps* ops) noexcept :
        Runtime(ops)
    {
    }

    GrapplerConfigs(const ::TFGrapplerConfigsOps* ops, const ::TFGrapplerConfigs* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_optimization_level(ice::sonic::GrapplerOptimizationLevel& out_level) const noexcept
    {
        ::TFGrapplerOptimizationLevel raw_out_level{};
        m_ops->get_optimization_level(get_handle(), &raw_out_level);
        out_level = ice::sonic::GrapplerOptimizationLevel{raw_out_level};
    }

    void set_optimization_level(ice::sonic::GrapplerOptimizationLevel level) const noexcept
    {
        m_ops->set_optimization_level(get_handle(), level.get_value());
    }

    void get_optimizer_configs(
        ice::sonic::GrapplerOptimizerConfigs* out_configs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_optimizer_configs(
            get_handle(),
            out_configs == nullptr ? nullptr : out_configs->get_handle(),
            out_status.get_handle()
        );
    }

    void set_optimizer_configs(
        const ice::sonic::GrapplerOptimizerConfigs* in_configs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_optimizer_configs(
            get_handle(),
            in_configs == nullptr ? nullptr : in_configs->get_handle(),
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
