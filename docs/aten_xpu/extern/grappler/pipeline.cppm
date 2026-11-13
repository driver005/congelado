module;

#include "cc/proto/graph.pb.h"

export module aten_xpu_extern_grappler:pipeline;

import std;
import :pass_context;
import :function_inlining_pass;
import :pruning_pass;
import :constant_folding_pass;
import :arithmetic_pass;
import :dependency_pass;
import :dead_branch_pass;
import :stack_push_removal_pass;
import :loop_invariant_motion_pass;
import :shape_pass;
import :layout_pass;
import :debug_stripper_pass;
import :pin_to_host_pass;
import :mixed_precision_pass;
import :swapping_pass;
import :recomputation_pass;
import :auto_parallel_pass;
import :scoped_allocator_pass;
import :batch_norm_folding_pass;
import :remapper;

export namespace aten_xpu {

// The plugin's graph passes in the order of TensorFlow's meta optimizer: prune first so the other
// passes see a small graph, simplify (constants, arithmetic, dependencies, static shapes), then the
// rewrites that target the device (batch norm folding, fusion), and prune again for the nodes they
// left behind.
class SyclGraphPipeline
{
public:
    explicit SyclGraphPipeline(SyclPassContext& context) :
        m_context{context},
        m_inlining{context},
        m_pruning{context},
        m_arithmetic{context},
        m_dependency{context},
        m_dead_branch{context},
        m_stack_push{context},
        m_loop_invariant{context},
        m_shape{context},
        m_layout{context},
        m_debug_stripper{context},
        m_pin_to_host{context},
        m_mixed_precision{context},
        m_swapping{context},
        m_recomputation{context},
        m_auto_parallel{context},
        m_scoped_allocator{context},
        m_batch_norm{context},
        m_remapper{context}
    {
    }

    SyclGraphPipeline(const SyclGraphPipeline&) = delete;
    SyclGraphPipeline& operator=(const SyclGraphPipeline&) = delete;
    SyclGraphPipeline(SyclGraphPipeline&&) = delete;
    SyclGraphPipeline& operator=(SyclGraphPipeline&&) = delete;

    void run(tensorflow::GraphDef& graph)
    {
        m_changed_passes = 0;
        m_changed_passes += step(SyclGrapplerPass::k_function, m_inlining, graph);
        m_changed_passes += step(SyclGrapplerPass::k_pruning, m_pruning, graph);
        m_changed_passes += step(SyclGrapplerPass::k_debug_stripper, m_debug_stripper, graph);
        m_changed_passes += step(SyclGrapplerPass::k_constant_folding, m_folding, graph);
        m_changed_passes += step(SyclGrapplerPass::k_arithmetic, m_arithmetic, graph);
        m_changed_passes += step(SyclGrapplerPass::k_dependency, m_dependency, graph);
        m_changed_passes += step(SyclGrapplerPass::k_loop, m_dead_branch, graph);
        m_changed_passes += step(SyclGrapplerPass::k_loop, m_loop_invariant, graph);
        m_changed_passes += step(SyclGrapplerPass::k_loop, m_stack_push, graph);
        m_changed_passes += step(SyclGrapplerPass::k_shape, m_shape, graph);
        m_changed_passes += step(SyclGrapplerPass::k_layout, m_layout, graph);
        m_changed_passes += m_batch_norm.run(graph);
        m_changed_passes += m_remapper.run(graph);
        m_changed_passes += step(SyclGrapplerPass::k_mixed_precision, m_mixed_precision, graph);
        m_changed_passes += step(SyclGrapplerPass::k_recomputation, m_recomputation, graph);
        m_changed_passes += step(SyclGrapplerPass::k_memory, m_swapping, graph);
        m_changed_passes += step(SyclGrapplerPass::k_auto_parallel, m_auto_parallel, graph);
        m_changed_passes += step(SyclGrapplerPass::k_scoped_allocator, m_scoped_allocator, graph);
        m_changed_passes += step(SyclGrapplerPass::k_pin_to_host, m_pin_to_host, graph);
        m_changed_passes += step(SyclGrapplerPass::k_pruning, m_pruning, graph);
    }

    std::size_t getChangedPassCount() const noexcept
    {
        return m_changed_passes;
    }

    std::size_t getFusedCount() const noexcept
    {
        return m_remapper.getFusedCount();
    }

private:
    // Runs the pass unless the host's settings switch its optimizer off.
    template<typename Pass>
    bool step(SyclGrapplerPass which, Pass& pass, tensorflow::GraphDef& graph)
    {
        return m_context.isEnabled(which) && pass.run(graph);
    }

    SyclPassContext& m_context;
    SyclFunctionInliningPass m_inlining;
    SyclPruningPass m_pruning;
    SyclConstantFoldingPass m_folding;
    SyclArithmeticPass m_arithmetic;
    SyclDependencyPass m_dependency;
    SyclDeadBranchPass m_dead_branch;
    SyclStackPushRemovalPass m_stack_push;
    SyclLoopInvariantMotionPass m_loop_invariant;
    SyclShapePass m_shape;
    SyclLayoutPass m_layout;
    SyclDebugStripperPass m_debug_stripper;
    SyclPinToHostPass m_pin_to_host;
    SyclMixedPrecisionPass m_mixed_precision;
    SyclSwappingPass m_swapping;
    SyclRecomputationPass m_recomputation;
    SyclAutoParallelPass m_auto_parallel;
    SyclScopedAllocatorPass m_scoped_allocator;
    SyclBatchNormFoldingPass m_batch_norm;
    SyclRemapper m_remapper;
    std::size_t m_changed_passes{0};
};

} // namespace aten_xpu
