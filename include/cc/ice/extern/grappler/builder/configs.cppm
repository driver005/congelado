// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"

export module cc_ice_extern_grappler_builder:configs;

import std;

export namespace ice::builder {

class TFGrapplerConfigsOps
{
public:
    static TFGrapplerConfigsOps* create(void* ctx) noexcept
    {
        return static_cast<TFGrapplerConfigsOps*>(ctx);
    }

    template<typename HandleT>
    static TFGrapplerConfigsOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFGrapplerConfigsOps*>(handle->plugin_data);
    }

    virtual ~TFGrapplerConfigsOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_optimization_level(TFGrapplerOptimizationLevel* out_level) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_optimization_level(TFGrapplerOptimizationLevel level) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_optimizer_configs(TFGrapplerOptimizerConfigs* out_configs) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_optimizer_configs(const TFGrapplerOptimizerConfigs* in_configs) noexcept = 0;

    static TFGrapplerConfigsOps* get_generic_vtable()
    {
        static TFGrapplerConfigsOps vtable = {
            .struct_size = TF_RAPPLERCONFIGS_STRUCT_SIZE,
            .get_optimization_level =
                [](TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel* out_level) noexcept
            {
                auto* self = TFGrapplerConfigsOps::create(configs);
                auto res = self->get_optimization_level(out_level);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_optimization_level =
                [](TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel level) noexcept
            {
                auto* self = TFGrapplerConfigsOps::create(configs);
                auto res = self->set_optimization_level(level);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_optimizer_configs =
                [](TFGrapplerConfigs* configs,
                   TFGrapplerOptimizerConfigs* out_configs,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerConfigsOps::create(configs);
                auto res = self->get_optimizer_configs(out_configs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_optimizer_configs =
                [](TFGrapplerConfigs* configs,
                   const TFGrapplerOptimizerConfigs* in_configs,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFGrapplerConfigsOps::create(configs);
                auto res = self->set_optimizer_configs(in_configs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
