module;

#include "include/c/intern/datatype.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:attention_sdpa_backward;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :attention_sdpa_selector;
import :attention_sdpa_math;

export namespace aten_xpu {

class SyclSdpaBackwardKernel : public SyclKernel<SyclSdpaBackwardKernel>
{
public:
    static constexpr std::string_view k_name = "ScaledDotProductAttentionGrad";

    SyclSdpaBackwardKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclSdpaBackwardKernel>{ops},
        m_scale{construction.getFloat("scale", 0.0F)},
        m_is_causal{construction.getBool("is_causal", false)}
    {
    }

    void compute(SyclKernelContext& context)
    {
        auto gradient_output = context.getInput(0);
        auto query = context.getInput(1);
        auto key = context.getInput(2);
        auto value = context.getInput(3);
        auto queue = context.getQueue();
        if (!gradient_output || !query || !key || !value || !queue) {
            context.propagate();
            return;
        }
        if (!SyclSdpaSelector::valid_shapes(query->get(), key->get(), value->get())) {
            context.fail(
                TF_INVALID_ARGUMENT,
                "attention expects [batch, heads, length, head_dim] inputs"
            );
            return;
        }
        auto mask_input = context.getInputCount() > 5 ? context.getInput(5) : std::nullopt;
        const auto mask =
            mask_input ? std::optional<std::reference_wrapper<const SyclTensor>>{mask_input->get()}
                       : std::nullopt;

        auto gradient_query =
            context.allocateOutput(0, query->get().getDtype(), query->get().getDims());
        auto gradient_key = context.allocateOutput(1, key->get().getDtype(), key->get().getDims());
        auto gradient_value =
            context.allocateOutput(2, value->get().getDtype(), value->get().getDims());
        if (!gradient_query || !gradient_key || !gradient_value) {
            context.propagate();
            return;
        }

        const float scale = m_scale > 0.0F
                                ? m_scale
                                : 1.0F / std::sqrt(static_cast<float>(query->get().getDims()[3]));
        SyclSdpaMath math{context, queue->get()};
        auto saved = context.getInputCount() > 4 ? context.getInput(4) : std::nullopt;
        if (!saved || saved->get().getByteSize() == 0) {
            auto recomputed_output =
                context.allocateTemp(query->get().getDtype(), gradient_output->get().getDims());
            if (!recomputed_output) {
                context.propagate();
                return;
            }
            saved = math.forward(
                query->get(),
                key->get(),
                value->get(),
                mask,
                recomputed_output->get(),
                scale,
                m_is_causal
            );
            if (!saved) {
                context.propagate();
                return;
            }
        }
        math.backward(
            gradient_output->get(),
            query->get(),
            key->get(),
            value->get(),
            saved->get(),
            gradient_query->get(),
            gradient_key->get(),
            gradient_value->get(),
            scale
        );
        context.propagate();
    }

private:
    float m_scale;
    bool m_is_causal;
};

} // namespace aten_xpu
