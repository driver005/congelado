// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/grappler/configs.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/grappler/configs.h"

export module cc_ice_extern_grappler_sonic:grappler_optimizer_configs;

import std;

export namespace ice::sonic {

class GrapplerOptimizerConfigs
{
public:
    GrapplerOptimizerConfigs() noexcept
    {
        m_value.struct_size = sizeof(::TFGrapplerOptimizerConfigs);
    }

    explicit GrapplerOptimizerConfigs(const ::TFGrapplerOptimizerConfigs& value) noexcept :
        m_value{value}
    {
    }

    GrapplerOptimizerConfigs(
        ::TF_TriState disable_model_pruning,
        ::TF_TriState implementation_selector,
        ::TF_TriState function_optimization,
        ::TF_TriState common_subgraph_elimination,
        ::TF_TriState arithmetic_optimization,
        ::TF_TriState debug_stripper,
        ::TF_TriState constant_folding,
        ::TF_TriState shape_optimization,
        ::TF_TriState auto_mixed_precision,
        ::TF_TriState auto_mixed_precision_onednn_bfloat16,
        ::TF_TriState auto_mixed_precision_mkl,
        ::TF_TriState pin_to_host_optimization,
        ::TF_TriState layout_optimizer,
        ::TF_TriState remapping,
        ::TF_TriState loop_optimization,
        ::TF_TriState dependency_optimization,
        ::TF_TriState auto_parallel,
        ::TF_TriState memory_optimization,
        ::TF_TriState scoped_allocator_optimization
    ) noexcept
    {
        m_value.struct_size = sizeof(::TFGrapplerOptimizerConfigs);
        m_value.disable_model_pruning = disable_model_pruning;
        m_value.implementation_selector = implementation_selector;
        m_value.function_optimization = function_optimization;
        m_value.common_subgraph_elimination = common_subgraph_elimination;
        m_value.arithmetic_optimization = arithmetic_optimization;
        m_value.debug_stripper = debug_stripper;
        m_value.constant_folding = constant_folding;
        m_value.shape_optimization = shape_optimization;
        m_value.auto_mixed_precision = auto_mixed_precision;
        m_value.auto_mixed_precision_onednn_bfloat16 = auto_mixed_precision_onednn_bfloat16;
        m_value.auto_mixed_precision_mkl = auto_mixed_precision_mkl;
        m_value.pin_to_host_optimization = pin_to_host_optimization;
        m_value.layout_optimizer = layout_optimizer;
        m_value.remapping = remapping;
        m_value.loop_optimization = loop_optimization;
        m_value.dependency_optimization = dependency_optimization;
        m_value.auto_parallel = auto_parallel;
        m_value.memory_optimization = memory_optimization;
        m_value.scoped_allocator_optimization = scoped_allocator_optimization;
    }

    ::TF_TriState getDisableModelPruning() const noexcept
    {
        return m_value.disable_model_pruning;
    }

    void setDisableModelPruning(::TF_TriState value) noexcept
    {
        m_value.disable_model_pruning = value;
    }

    ::TF_TriState getImplementationSelector() const noexcept
    {
        return m_value.implementation_selector;
    }

    void setImplementationSelector(::TF_TriState value) noexcept
    {
        m_value.implementation_selector = value;
    }

    ::TF_TriState getFunctionOptimization() const noexcept
    {
        return m_value.function_optimization;
    }

    void setFunctionOptimization(::TF_TriState value) noexcept
    {
        m_value.function_optimization = value;
    }

    ::TF_TriState getCommonSubgraphElimination() const noexcept
    {
        return m_value.common_subgraph_elimination;
    }

    void setCommonSubgraphElimination(::TF_TriState value) noexcept
    {
        m_value.common_subgraph_elimination = value;
    }

    ::TF_TriState getArithmeticOptimization() const noexcept
    {
        return m_value.arithmetic_optimization;
    }

    void setArithmeticOptimization(::TF_TriState value) noexcept
    {
        m_value.arithmetic_optimization = value;
    }

    ::TF_TriState getDebugStripper() const noexcept
    {
        return m_value.debug_stripper;
    }

    void setDebugStripper(::TF_TriState value) noexcept
    {
        m_value.debug_stripper = value;
    }

    ::TF_TriState getConstantFolding() const noexcept
    {
        return m_value.constant_folding;
    }

    void setConstantFolding(::TF_TriState value) noexcept
    {
        m_value.constant_folding = value;
    }

    ::TF_TriState getShapeOptimization() const noexcept
    {
        return m_value.shape_optimization;
    }

    void setShapeOptimization(::TF_TriState value) noexcept
    {
        m_value.shape_optimization = value;
    }

    ::TF_TriState getAutoMixedPrecision() const noexcept
    {
        return m_value.auto_mixed_precision;
    }

    void setAutoMixedPrecision(::TF_TriState value) noexcept
    {
        m_value.auto_mixed_precision = value;
    }

    ::TF_TriState getAutoMixedPrecisionOnednnBfloat16() const noexcept
    {
        return m_value.auto_mixed_precision_onednn_bfloat16;
    }

    void setAutoMixedPrecisionOnednnBfloat16(::TF_TriState value) noexcept
    {
        m_value.auto_mixed_precision_onednn_bfloat16 = value;
    }

    ::TF_TriState getAutoMixedPrecisionMkl() const noexcept
    {
        return m_value.auto_mixed_precision_mkl;
    }

    void setAutoMixedPrecisionMkl(::TF_TriState value) noexcept
    {
        m_value.auto_mixed_precision_mkl = value;
    }

    ::TF_TriState getPinToHostOptimization() const noexcept
    {
        return m_value.pin_to_host_optimization;
    }

    void setPinToHostOptimization(::TF_TriState value) noexcept
    {
        m_value.pin_to_host_optimization = value;
    }

    ::TF_TriState getLayoutOptimizer() const noexcept
    {
        return m_value.layout_optimizer;
    }

    void setLayoutOptimizer(::TF_TriState value) noexcept
    {
        m_value.layout_optimizer = value;
    }

    ::TF_TriState getRemapping() const noexcept
    {
        return m_value.remapping;
    }

    void setRemapping(::TF_TriState value) noexcept
    {
        m_value.remapping = value;
    }

    ::TF_TriState getLoopOptimization() const noexcept
    {
        return m_value.loop_optimization;
    }

    void setLoopOptimization(::TF_TriState value) noexcept
    {
        m_value.loop_optimization = value;
    }

    ::TF_TriState getDependencyOptimization() const noexcept
    {
        return m_value.dependency_optimization;
    }

    void setDependencyOptimization(::TF_TriState value) noexcept
    {
        m_value.dependency_optimization = value;
    }

    ::TF_TriState getAutoParallel() const noexcept
    {
        return m_value.auto_parallel;
    }

    void setAutoParallel(::TF_TriState value) noexcept
    {
        m_value.auto_parallel = value;
    }

    ::TF_TriState getMemoryOptimization() const noexcept
    {
        return m_value.memory_optimization;
    }

    void setMemoryOptimization(::TF_TriState value) noexcept
    {
        m_value.memory_optimization = value;
    }

    ::TF_TriState getScopedAllocatorOptimization() const noexcept
    {
        return m_value.scoped_allocator_optimization;
    }

    void setScopedAllocatorOptimization(::TF_TriState value) noexcept
    {
        m_value.scoped_allocator_optimization = value;
    }

    ::TFGrapplerOptimizerConfigs* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TFGrapplerOptimizerConfigs* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TFGrapplerOptimizerConfigs m_value{};
};

} // namespace ice::sonic
