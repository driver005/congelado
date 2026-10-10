// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_grappler_sonic:configs;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGrapplerConfigsOps : public ice::sonic::Runtime<::TFGrapplerConfigsOps, ::TFGrapplerConfigs>
{
public:
    TFGrapplerConfigsOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFGrapplerConfigsOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFGrapplerConfigs* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFGrapplerConfigsOps(const ::TFGrapplerConfigsOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGrapplerConfigsOps(const ::TFGrapplerConfigsOps* ops, ::TFGrapplerConfigs* handle) noexcept :
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

    void get_optimization_level(TFGrapplerOptimizationLevel* out_level) const noexcept
    {
        m_ops->get_optimization_level(get_handle(), out_level);
    }

    void set_optimization_level(TFGrapplerOptimizationLevel level) const noexcept
    {
        m_ops->set_optimization_level(get_handle(), level);
    }

    void get_optimizer_configs(
        TFGrapplerOptimizerConfigs* out_configs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_optimizer_configs(get_handle(), out_configs, out_status.get_handle());
    }

    void set_optimizer_configs(
        const TFGrapplerOptimizerConfigs* in_configs,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_optimizer_configs(get_handle(), in_configs, out_status.get_handle());
    }
};

} // namespace ice::sonic
