// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_grappler_builder:configs;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFGrapplerConfigsOps
{
public:
    explicit TFGrapplerConfigsOps(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    TFGrapplerConfigsOps(const TFGrapplerConfigsOps&) = delete;
    TFGrapplerConfigsOps& operator=(const TFGrapplerConfigsOps&) = delete;

    static TFGrapplerConfigsOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFGrapplerConfigsOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerConfigsOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFGrapplerConfigsOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerConfigsOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_optimization_level(TFGrapplerOptimizationLevel* out_level) noexcept = 0;
    virtual void set_optimization_level(TFGrapplerOptimizationLevel level) noexcept = 0;
    virtual void get_optimizer_configs(
        TFGrapplerOptimizerConfigs* out_configs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_optimizer_configs(
        const TFGrapplerOptimizerConfigs* in_configs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerConfigs*)) noexcept
    {
        m_vtable = ::TFGrapplerConfigsOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerConfigsOps, set_optimizer_configs),

            .create = create,
            .destroy =
                [](TFGrapplerConfigs* handle) noexcept
            {
                auto& self = TFGrapplerConfigsOps::from_handle(handle);
                self.destroy();
            },
            .get_optimization_level =
                [](TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel* out_level) noexcept
            {
                auto& self = TFGrapplerConfigsOps::from_handle(configs);
                self.get_optimization_level(out_level);
            },
            .set_optimization_level =
                [](TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel level) noexcept
            {
                auto& self = TFGrapplerConfigsOps::from_handle(configs);
                self.set_optimization_level(level);
            },
            .get_optimizer_configs =
                [](TFGrapplerConfigs* configs,
                   TFGrapplerOptimizerConfigs* out_configs,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerConfigsOps::from_handle(configs);
                self.get_optimizer_configs(
                    out_configs,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_optimizer_configs =
                [](TFGrapplerConfigs* configs,
                   const TFGrapplerOptimizerConfigs* in_configs,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFGrapplerConfigsOps::from_handle(configs);
                self.set_optimizer_configs(
                    in_configs,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    const ::TFGrapplerConfigsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFGrapplerConfigs& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFGrapplerConfigsOps*>(&m_vtable)
        );
    }

private:
    ::TFGrapplerConfigsOps m_vtable;
    ::TFGrapplerConfigs m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
