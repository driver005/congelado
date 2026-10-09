module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_extern_kernel:matmul_addmm;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :matmul_scaled_add;
import :onednn_matmul_primitive;

export namespace aten_xpu {

class SyclAddmmKernel : public SyclKernel<SyclAddmmKernel>
{
public:
    static constexpr std::string_view k_name = "Addmm";

    SyclAddmmKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclAddmmKernel>{ops},
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

        m_scratch_dims.assign({first->get().getDims()[0], second->get().getDims()[1]});
        auto output = context.allocateOutput(0, first->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }
        SyclMatmulPrimitive::run(
            queue->get(),
            first->get(),
            second->get(),
            std::nullopt,
            output->get(),
            SyclScaledAdd::build(m_alpha, m_beta, addend->get(), m_activation)
        );
    }

private:
    float m_alpha;
    float m_beta;
    int64_t m_activation;
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
