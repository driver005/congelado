// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_extern_grappler_builder:configs;

import std;
import cc_ice_extern_grappler_sonic;
import cc_ice_intern_sonic;

export namespace ice::builder {

class GrapplerConfigs
{
public:
    explicit GrapplerConfigs(const ::TF_StatusOps* Status_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
    }

    GrapplerConfigs(const GrapplerConfigs&) = delete;
    GrapplerConfigs& operator=(const GrapplerConfigs&) = delete;

    static GrapplerConfigs& from_handle(void* ctx) noexcept
    {
        return *static_cast<GrapplerConfigs*>(ctx);
    }

    template<typename HandleT>
    static GrapplerConfigs& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<GrapplerConfigs*>(handle->plugin_data);
    }

    virtual ~GrapplerConfigs() = default;
    virtual void destroy() noexcept = 0;
    virtual void
    get_optimization_level(ice::sonic::GrapplerOptimizationLevel& out_level) noexcept = 0;
    virtual void set_optimization_level(ice::sonic::GrapplerOptimizationLevel level) noexcept = 0;
    virtual void get_optimizer_configs(
        ice::sonic::GrapplerOptimizerConfigs* out_configs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_optimizer_configs(
        const ice::sonic::GrapplerOptimizerConfigs* in_configs,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFGrapplerConfigs*)) noexcept
    {
        m_vtable = ::TFGrapplerConfigsOps{
            .struct_size = TF_OFFSET_OF_END(::TFGrapplerConfigsOps, set_optimizer_configs),

            .create = create,
            .destroy =
                [](::TFGrapplerConfigs* handle) noexcept
            {
                auto& self = GrapplerConfigs::from_handle(handle);
                self.destroy();
            },
            .get_optimization_level =
                [](::TFGrapplerConfigs* configs, ::TFGrapplerOptimizationLevel* out_level) noexcept
            {
                auto& self = GrapplerConfigs::from_handle(configs);
                ice::sonic::GrapplerOptimizationLevel wrapped_out_level{*out_level};
                self.get_optimization_level(wrapped_out_level);
                *out_level = wrapped_out_level.get_value();
            },
            .set_optimization_level =
                [](::TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel level) noexcept
            {
                auto& self = GrapplerConfigs::from_handle(configs);
                self.set_optimization_level(ice::sonic::GrapplerOptimizationLevel{level});
            },
            .get_optimizer_configs =
                [](::TFGrapplerConfigs* configs,
                   ::TFGrapplerOptimizerConfigs* out_configs,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerConfigs::from_handle(configs);
                std::optional<ice::sonic::GrapplerOptimizerConfigs> wrapped_out_configs;
                if (out_configs != nullptr) {
                    wrapped_out_configs.emplace(*out_configs);
                }
                self.get_optimizer_configs(
                    wrapped_out_configs ? &*wrapped_out_configs : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
                if (out_configs != nullptr) {
                    *out_configs = *wrapped_out_configs->get_handle();
                }
            },
            .set_optimizer_configs =
                [](::TFGrapplerConfigs* configs,
                   const ::TFGrapplerOptimizerConfigs* in_configs,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = GrapplerConfigs::from_handle(configs);
                std::optional<ice::sonic::GrapplerOptimizerConfigs> wrapped_in_configs;
                if (in_configs != nullptr) {
                    wrapped_in_configs.emplace(*in_configs);
                }
                self.set_optimizer_configs(
                    wrapped_in_configs ? &*wrapped_in_configs : nullptr,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, handle};
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
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFGrapplerConfigsOps*>(&m_vtable));
    }

private:
    ::TFGrapplerConfigsOps m_vtable;
    ::TFGrapplerConfigs m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};
};

} // namespace ice::builder
