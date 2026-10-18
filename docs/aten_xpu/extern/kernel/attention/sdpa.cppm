module;

#include "include/c/intern/datatype.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:attention_sdpa;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :attention_sdpa_selector;
import :attention_sdpa_math;
import :attention_onednn_sdpa;

export namespace aten_xpu {

inline constexpr std::string_view k_sdpa_kernel_name = "ScaledDotProductAttention";

class SyclSdpaKernel : public SyclKernel<SyclSdpaKernel>
{
public:
    SyclSdpaKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclSdpaKernel>{ops, k_sdpa_kernel_name},
        m_scale{construction.getFloat("scale", 0.0F)},
        m_dropout{construction.getFloat("dropout", 0.0F)},
        m_is_causal{construction.getBool("is_causal", false)},
        m_return_probabilities{construction.getBool("return_probabilities", false)}
    {
        if (m_dropout > 0.0F) {
            construction.fail(
                ice::sonic::Code::k_unimplemented,
                "attention dropout is not supported"
            );
        }
    }

    void compute(SyclKernelContext& context)
    {
        auto query = context.getInput(0);
        auto key = context.getInput(1);
        auto value = context.getInput(2);
        auto queue = context.getQueue();
        if (!query || !key || !value || !queue) {
            context.propagate();
            return;
        }
        auto mask_input = context.getInputCount() > 3 ? context.getInput(3) : std::nullopt;
        const auto mask =
            mask_input ? std::optional<std::reference_wrapper<const SyclTensor>>{mask_input->get()}
                       : std::nullopt;

        auto backend =
            m_selector.choose(query->get(), key->get(), value->get(), mask, m_dropout, m_is_causal);
        if (backend == m_selector.Backend::invalid) {
            context.fail(
                ice::sonic::Code::k_invalid_argument,
                "attention expects [batch, heads, length, head_dim] inputs"
            );
            return;
        }
        if (m_return_probabilities) {
            backend = m_selector.Backend::math;
        }

        m_scratch_dims.assign(query->get().getDims().begin(), query->get().getDims().end());
        m_scratch_dims[3] = value->get().getDims()[3];
        auto output = context.allocateOutput(0, query->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        const float scale = m_scale > 0.0F
                                ? m_scale
                                : 1.0F / std::sqrt(static_cast<float>(query->get().getDims()[3]));
        if (backend == m_selector.Backend::fused) {
            getRuntime().getSdpa().run(
                queue->get(),
                query->get(),
                key->get(),
                value->get(),
                mask,
                output->get(),
                scale
            );
            return;
        }

        SyclSdpaMath math{getRuntime(), context, queue->get()};
        auto probabilities = math.forward(
            query->get(),
            key->get(),
            value->get(),
            mask,
            output->get(),
            scale,
            m_is_causal
        );
        if (!probabilities) {
            context.propagate();
            return;
        }
        if (m_return_probabilities) {
            auto saved = context.allocateOutput(
                1,
                ice::sonic::DataTypeEnum::k_float,
                probabilities->get().getDims()
            );
            if (saved) {
                queue->get().memcpy(
                    saved->get().getData(),
                    probabilities->get().getData(),
                    probabilities->get().getByteSize()
                );
            }
        }
    }

private:
    float m_scale;
    float m_dropout;
    bool m_is_causal;
    bool m_return_probabilities;
    std::vector<int64_t> m_scratch_dims;
    SyclSdpaSelector m_selector;
};

} // namespace aten_xpu
