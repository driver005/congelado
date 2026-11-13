module;

#include "include/c/extern/grappler/configs.h"

export module aten_xpu_extern_grappler:pass_context;

import std;
import :properties;
import :op_lookup;

export namespace aten_xpu {

// The optimizers that have a plugin pass, named after the settings that switch them.
enum class SyclGrapplerPass
{
    k_pruning,
    k_constant_folding,
    k_arithmetic,
    k_dependency,
    k_shape,
    k_function,
    k_loop,
    k_debug_stripper,
    k_pin_to_host,
    k_mixed_precision,
    k_layout,
    k_memory,
    k_recomputation,
    k_scoped_allocator,
    k_auto_parallel
};

// What every graph pass shares: the nodes it must keep, and the host's shape and op information.
class SyclPassContext
{
public:
    SyclPassContext(SyclGraphProperties& properties, SyclOpLookup& lookup) noexcept :
        m_properties{properties},
        m_lookup{lookup}
    {
    }

    SyclPassContext(const SyclPassContext&) = delete;
    SyclPassContext& operator=(const SyclPassContext&) = delete;
    SyclPassContext(SyclPassContext&&) = delete;
    SyclPassContext& operator=(SyclPassContext&&) = delete;

    void setPreserved(std::set<std::string> preserved)
    {
        m_preserved = std::move(preserved);
    }

    void setFetch(std::set<std::string> fetch)
    {
        m_fetch = std::move(fetch);
    }

    void setConfigs(const ::TFGrapplerOptimizerConfigs& configs)
    {
        m_configs = configs;
        m_scoped_ops.clear();
        std::string_view names = configs.scoped_allocator_enable_ops == nullptr
                                     ? "CollectiveReduce"
                                     : configs.scoped_allocator_enable_ops;
        while (!names.empty()) {
            const auto comma = names.find(',');
            const auto name = names.substr(0, comma);
            if (!name.empty()) {
                m_scoped_ops.emplace(name);
            }
            names = comma == std::string_view::npos ? std::string_view{} : names.substr(comma + 1);
        }
    }

    // The device facts the memory and parallelism passes budget with.
    void setDeviceInfo(
        std::uint64_t memory_bytes,
        std::uint64_t bandwidth,
        int device_count
    ) noexcept
    {
        m_device_memory_bytes = memory_bytes;
        m_device_bandwidth = bandwidth;
        m_device_count = device_count;
    }

    // The host's memory budget, or the device's memory size when it set none.
    std::uint64_t getMemoryLimitBytes() const noexcept
    {
        return m_configs.memory_limit_bytes != 0 ? m_configs.memory_limit_bytes
                                                 : m_device_memory_bytes;
    }

    std::uint64_t getDeviceBandwidth() const noexcept
    {
        return m_device_bandwidth;
    }

    int getDeviceCount() const noexcept
    {
        return m_device_count;
    }

    int getReplicaCount() const noexcept
    {
        return m_configs.auto_parallel_replicas > 0 ? m_configs.auto_parallel_replicas : 1;
    }

    const std::set<std::string>& getScopedAllocatorOps() const noexcept
    {
        return m_scoped_ops;
    }

    // The host's setting for the optimizer, or TensorFlow's default when the host left it alone:
    // everything runs except the debug stripper, pin to host and mixed precision.
    bool isEnabled(SyclGrapplerPass pass) const noexcept
    {
        switch (pass) {
            case SyclGrapplerPass::k_pruning:
                return m_configs.disable_model_pruning != ::TF_TriState_On;
            case SyclGrapplerPass::k_constant_folding:
                return on(m_configs.constant_folding, true);
            case SyclGrapplerPass::k_arithmetic:
                return on(m_configs.arithmetic_optimization, true);
            case SyclGrapplerPass::k_dependency:
                return on(m_configs.dependency_optimization, true);
            case SyclGrapplerPass::k_shape:
                return on(m_configs.shape_optimization, true);
            case SyclGrapplerPass::k_function:
                return on(m_configs.function_optimization, true);
            case SyclGrapplerPass::k_loop:
                return on(m_configs.loop_optimization, true);
            case SyclGrapplerPass::k_debug_stripper:
                return on(m_configs.debug_stripper, false);
            case SyclGrapplerPass::k_pin_to_host:
                return on(m_configs.pin_to_host_optimization, false);
            case SyclGrapplerPass::k_layout:
                return on(m_configs.layout_optimizer, true);
            case SyclGrapplerPass::k_recomputation:
                // TensorFlow runs recomputation only when asked for; swapping is part of the
                // default.
                return m_configs.memory_optimization == ::TF_TriState_On;
            case SyclGrapplerPass::k_memory:
                return on(m_configs.memory_optimization, true);
            case SyclGrapplerPass::k_scoped_allocator:
                return on(m_configs.scoped_allocator_optimization, false);
            case SyclGrapplerPass::k_auto_parallel:
                return on(m_configs.auto_parallel, false);
            case SyclGrapplerPass::k_mixed_precision:
                return on(m_configs.auto_mixed_precision, false) ||
                       on(m_configs.auto_mixed_precision_onednn_bfloat16, false);
        }
        return false;
    }

    // The low precision type: bfloat16 when its setting is on, half otherwise.
    bool prefersBfloat16() const noexcept
    {
        return m_configs.auto_mixed_precision_onednn_bfloat16 == ::TF_TriState_On;
    }

    // Feeds, fetches, init ops and anything else the host marked untouchable.
    bool isPreserved(const std::string& name) const
    {
        return m_preserved.contains(name) || m_fetch.contains(name);
    }

    const std::set<std::string>& getPreserved() const noexcept
    {
        return m_preserved;
    }

    const std::set<std::string>& getFetch() const noexcept
    {
        return m_fetch;
    }

    SyclGraphProperties& getProperties() noexcept
    {
        return m_properties;
    }

    SyclOpLookup& getLookup() noexcept
    {
        return m_lookup;
    }

private:
    static bool on(::TF_TriState value, bool by_default) noexcept
    {
        return value == ::TF_TriState_Default ? by_default : value == ::TF_TriState_On;
    }

    SyclGraphProperties& m_properties;
    SyclOpLookup& m_lookup;
    std::set<std::string> m_preserved;
    std::set<std::string> m_fetch;
    ::TFGrapplerOptimizerConfigs m_configs{};
    std::set<std::string> m_scoped_ops{"CollectiveReduce"};
    std::uint64_t m_device_memory_bytes{0};
    std::uint64_t m_device_bandwidth{0};
    int m_device_count{0};
};

} // namespace aten_xpu
