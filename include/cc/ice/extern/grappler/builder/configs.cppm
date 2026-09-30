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
    TFGrapplerConfigsOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_optimization_level(TFGrapplerOptimizationLevel* out_level) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_optimization_level(TFGrapplerOptimizationLevel level) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_optimizer_configs(TFGrapplerOptimizerConfigs* out_configs) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_optimizer_configs(const TFGrapplerOptimizerConfigs* in_configs) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFGrapplerConfigsOps{
            .struct_size = TF_RAPPLERCONFIGS_STRUCT_SIZE,
            .get_optimization_level =
                [](TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel* out_level) noexcept
            {
                auto res =
                    TFGrapplerConfigsOps::from_handle(configs).get_optimization_level(out_level);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_optimization_level =
                [](TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel level) noexcept
            {
                auto res = TFGrapplerConfigsOps::from_handle(configs).set_optimization_level(level);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_optimizer_configs =
                [](TFGrapplerConfigs* configs,
                   TFGrapplerOptimizerConfigs* out_configs,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGrapplerConfigsOps::from_handle(configs).get_optimizer_configs(out_configs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_optimizer_configs =
                [](TFGrapplerConfigs* configs,
                   const TFGrapplerOptimizerConfigs* in_configs,
                   TF_Status* out_status) noexcept
            {
                auto res =
                    TFGrapplerConfigsOps::from_handle(configs).set_optimizer_configs(in_configs);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFGrapplerConfigsOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFGrapplerConfigs& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFGrapplerConfigsOps m_vtable;
    TFGrapplerConfigs m_handle;
};

} // namespace ice::builder
