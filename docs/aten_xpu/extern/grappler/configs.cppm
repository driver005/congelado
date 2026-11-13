module;

#include "include/c/extern/grappler/configs.h"

export module aten_xpu_extern_grappler:configs;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_grappler_builder;
import cc_ice_extern_grappler_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// What the plugin asks the host's built-in grappler to do, and where the host's own optimizer
// settings land: set_optimizer_configs stores them in the ops table for the plugin's passes. The
// plugin fuses contractions itself (SyclRemapper), so the host's remapper is switched off: it would
// rewrite the same chains into fused ops the plugin has no kernels for. Every other built-in
// optimizer keeps its default.
class SyclGrapplerConfigs : public ice::builder::GrapplerConfigs
{
public:
    explicit SyclGrapplerConfigs(SyclOpsTable& ops) noexcept :
        ice::builder::GrapplerConfigs{ops.getStatusOps()},
        m_ops{ops},
        m_status{ops}
    {
        m_configs.setRemapping(::TF_TriState_Off);
    }

    ~SyclGrapplerConfigs() override = default;
    SyclGrapplerConfigs(const SyclGrapplerConfigs&) = delete;
    SyclGrapplerConfigs& operator=(const SyclGrapplerConfigs&) = delete;
    SyclGrapplerConfigs(SyclGrapplerConfigs&&) = delete;
    SyclGrapplerConfigs& operator=(SyclGrapplerConfigs&&) = delete;

    void destroy() noexcept override
    {
        delete this;
    }

    void get_optimization_level(ice::sonic::GrapplerOptimizationLevel& out_level) noexcept override
    {
        out_level = m_level;
    }

    void set_optimization_level(ice::sonic::GrapplerOptimizationLevel level) noexcept override
    {
        m_level = level;
    }

    void get_optimizer_configs(
        ice::sonic::GrapplerOptimizerConfigs* out_configs,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (out_configs == nullptr) {
            m_status.fail(out_status, ice::sonic::Code::k_invalid_argument, "null configs");
            return;
        }
        *out_configs = m_configs;
    }

    void set_optimizer_configs(
        const ice::sonic::GrapplerOptimizerConfigs* in_configs,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (in_configs == nullptr) {
            m_status.fail(out_status, ice::sonic::Code::k_invalid_argument, "null configs");
            return;
        }
        m_configs = *in_configs;
        m_ops.setGrapplerOptimizerConfigs(*m_configs.get_handle());
    }

private:
    SyclOpsTable& m_ops;
    SyclStatus m_status;
    ice::sonic::GrapplerOptimizationLevel m_level{ice::sonic::GrapplerOptimizationLevel::k_l1};
    ice::sonic::GrapplerOptimizerConfigs m_configs;
};

} // namespace aten_xpu
