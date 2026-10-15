module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_extern_kernel:matmul_baddbmm;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :matmul_scaled_add;
import :onednn_matmul_primitive;

export namespace aten_xpu {

inline constexpr std::string_view k_baddbmm_kernel_name = "Baddbmm";

class SyclBaddbmmKernel : public SyclKernel<SyclBaddbmmKernel>
{
public:
    SyclBaddbmmKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclBaddbmmKernel>{ops, k_baddbmm_kernel_name},
        m_alpha{construction.getFloat("alpha", 1.0F)},
        m_beta{construction.getFloat("beta", 1.0F)},
        m_activation{construction.getInt64("activation", 0)}
    {
    }

    void compute(SyclKernelContext& context)
    {
        auto addend = context.getInput(0);
        auto first = context.getInput(1);
        auto second = context.getInput(2);
        auto queue = context.getQueue();
        if (!addend || !first || !second || !queue) {
            context.propagate();
            return;
        }

        const auto& first_dims = first->get().getDims();
        const auto& second_dims = second->get().getDims();
        m_scratch_dims.assign({first_dims[0], first_dims[1], second_dims[2]});
        auto output = context.allocateOutput(0, first->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }
        getRuntime().getMatmul().run(
            queue->get(),
            first->get(),
            second->get(),
            std::nullopt,
            output->get(),
            m_scaled_add.build(m_alpha, m_beta, addend->get(), m_activation)
        );
    }

private:
    float m_alpha;
    float m_beta;
    int64_t m_activation;
    std::vector<int64_t> m_scratch_dims;
    SyclScaledAdd m_scaled_add;
};

} // namespace aten_xpu
