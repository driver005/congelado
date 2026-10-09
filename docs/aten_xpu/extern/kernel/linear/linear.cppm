module;

#include "include/c/intern/datatype.h"
#include "include/c/intern/tensor.h"

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:linear;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :onednn_post_op_attributes;
import :onednn_fusion;
import :onednn_matmul_primitive;

export namespace aten_xpu {

class SyclLinearKernel : public SyclKernel<SyclLinearKernel>
{
public:
    static constexpr std::string_view k_name = "Linear";
    static constexpr std::size_t k_max_scalars = 4;

    SyclLinearKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclLinearKernel>{ops},
        m_unary{construction.getString("unary", "none")},
        m_algorithm{construction.getString("algorithm", "none")},
        m_binary{construction.getString("binary", "none")}
    {
        m_scalar_count = construction.getFloatList("scalars", m_scalars);
        if (m_unary == "none") {
            m_unary = SyclFusion::activation_name(construction.getInt64("activation", 0));
        }
    }

    void compute(SyclKernelContext& context)
    {
        auto input = context.getInput(0);
        auto weight = context.getInput(1);
        auto queue = context.getQueue();
        if (!input || !weight || !queue) {
            context.propagate();
            return;
        }
        auto bias = context.getInputCount() > 2 ? context.getInput(2) : std::nullopt;
        auto other = context.getInputCount() > 3 ? context.getInput(3) : std::nullopt;

        const auto& input_dims = input->get().getDims();
        const int64_t in_features = input_dims.back();
        const int64_t rows = input->get().element_count() / std::max<int64_t>(in_features, 1);
        const int64_t out_features = weight->get().getDims()[0];

        m_scratch_dims.assign(input_dims.begin(), input_dims.end());
        m_scratch_dims.back() = out_features;
        auto output = context.allocateOutput(0, input->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        auto& input_matrix = as_matrix(input->get(), rows, in_features, context);
        auto& output_matrix = as_matrix(output->get(), rows, out_features, context);
        std::optional<std::reference_wrapper<SyclTensor>> other_matrix;
        if (other) {
            other_matrix = as_matrix(other->get(), rows, out_features, context);
        }

        SyclPostOpAttributes attributes;
        const auto scalars = std::span<const float>{m_scalars}.first(m_scalar_count);
        const bool built = m_binary == "none"
                               ? SyclFusion::add_unary(attributes, m_unary, scalars, m_algorithm)
                               : add_binary(attributes, other_matrix);
        if (!built) {
            context.fail(TF_INVALID_ARGUMENT, "unsupported linear post op");
        } else {
            SyclMatmulPrimitive::run(
                queue->get(),
                input_matrix,
                weight->get(),
                bias ? std::optional<std::reference_wrapper<const SyclTensor>>{bias->get()}
                     : std::nullopt,
                output_matrix,
                attributes,
                true
            );
        }

        if (other_matrix) {
            other_matrix->get().destroy();
        }
        output_matrix.destroy();
        input_matrix.destroy();
    }

private:
    bool add_binary(
        SyclPostOpAttributes& attributes,
        std::optional<std::reference_wrapper<SyclTensor>> other
    ) const
    {
        if (!other) {
            return false;
        }
        static constexpr std::array<std::pair<std::string_view, dnnl::algorithm>, 6> k_binary{{
            {"add", dnnl::algorithm::binary_add},
            {"sum", dnnl::algorithm::binary_add},
            {"mul", dnnl::algorithm::binary_mul},
            {"sub", dnnl::algorithm::binary_sub},
            {"div", dnnl::algorithm::binary_div},
            {"max", dnnl::algorithm::binary_max},
        }};
        const auto found = std::ranges::find(
            k_binary,
            std::string_view{m_binary},
            &std::pair<std::string_view, dnnl::algorithm>::first
        );
        if (found == k_binary.end()) {
            return false;
        }
        attributes.addBinary(found->second, other->get(), true);
        return true;
    }

    static SyclTensor&
    as_matrix(SyclTensor& tensor, int64_t rows, int64_t columns, SyclKernelContext& context)
    {
        const std::array<int64_t, 2> dims{rows, columns};
        const std::array<int64_t, 2> strides{columns, 1};
        ::TF_Tensor* handle = nullptr;
        tensor.tensor_view(
            dims.data(),
            2,
            strides.data(),
            tensor.getStorageOffset(),
            &handle,
            context.getStatus()
        );
        return SyclHandle::resolve_raw<SyclTensor>(handle);
    }

    std::string m_unary;
    std::string m_algorithm;
    std::string m_binary;
    std::array<float, k_max_scalars> m_scalars{};
    std::size_t m_scalar_count{0};
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
