module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

export module aten_xpu_extern_kernel:matmul_addmv;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :matmul_scaled_add;
import :onednn_matmul_primitive;

export namespace aten_xpu {

class SyclAddmvKernel : public SyclKernel<SyclAddmvKernel>
{
public:
    static constexpr std::string_view k_name = "Addmv";

    SyclAddmvKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclAddmvKernel>{ops},
        m_alpha{construction.getFloat("alpha", 1.0F)},
        m_beta{construction.getFloat("beta", 1.0F)},
        m_activation{construction.getInt64("activation", 0)}
    {
    }

    void compute(SyclKernelContext& context)
    {

        auto addend = context.getInput(0);
        auto matrix = context.getInput(1);
        auto vector = context.getInput(2);
        auto queue = context.getQueue();
        if (!addend || !matrix || !vector || !queue) {
            context.propagate();
            return;
        }

        auto& column = as_column(vector->get(), context);
        auto& addend_column = as_column(addend->get(), context);
        m_scratch_dims.assign({matrix->get().getDims()[0]});
        auto output = context.allocateOutput(0, matrix->get().getDtype(), m_scratch_dims);
        if (output) {
            auto& output_column = as_column(output->get(), context);
            SyclMatmulPrimitive::run(
                queue->get(),
                matrix->get(),
                column,
                std::nullopt,
                output_column,
                SyclScaledAdd::build(m_alpha, m_beta, addend_column, m_activation)
            );
            output_column.destroy();
        }
        addend_column.destroy();
        column.destroy();
        context.propagate();

    }

private:
    static SyclTensor& as_column(SyclTensor& vector, SyclKernelContext& context)
    {

        const std::array<int64_t, 2> dims{vector.element_count(), 1};
        const std::array<int64_t, 2> strides{vector.getStrides().empty() ? 0 : vector.getStrides()[0], 1};
        ::TF_Tensor* handle = nullptr;
        vector.tensor_view(dims.data(), 2, strides.data(), vector.getStorageOffset(), &handle, context.getStatus());
        return SyclHandle::resolve_raw<SyclTensor>(handle);

    }

    float m_alpha;
    float m_beta;
    int64_t m_activation;
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
