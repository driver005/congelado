module;

#include "include/c/intern/datatype.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:quantized_woq_matmul;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :onednn_memory_layout;
import :onednn_primitive_executor;

export namespace aten_xpu {

class SyclWeightOnlyMatmulKernel : public SyclKernel<SyclWeightOnlyMatmulKernel>
{
public:
    static constexpr std::string_view k_name = "WeightOnlyQuantizedMatMul";

    SyclWeightOnlyMatmulKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclWeightOnlyMatmulKernel>{ops},
        m_group_size{construction.getInt64("group_size", 128)},
        m_symmetric{construction.getBool("symmetric", false)}
    {
    }

    void compute(SyclKernelContext& context)
    {
        auto activation = context.getInput(0);
        auto packed_weight = context.getInput(1);
        auto scale = context.getInput(2);
        auto queue = context.getQueue();
        if (!activation || !packed_weight || !scale || !queue) {
            context.propagate();
            return;
        }
        auto zero_point =
            m_symmetric || context.getInputCount() <= 3 ? std::nullopt : context.getInput(3);
        auto bias = context.getInputCount() > 4 ? context.getInput(4) : std::nullopt;

        const auto& activation_dims = activation->get().getDims();
        const int64_t inner = activation_dims.back();
        const int64_t rows = activation->get().element_count() / std::max<int64_t>(inner, 1);
        const int64_t columns = scale->get().getDims().back();
        const int64_t groups = (inner + m_group_size - 1) / m_group_size;

        m_scratch_dims.assign(activation_dims.begin(), activation_dims.end());
        m_scratch_dims.back() = columns;
        auto output = context.allocateOutput(0, activation->get().getDtype(), m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        const auto activation_type =
            SyclOnednnLayout::data_type(activation->get().getDtype()).value();
        const dnnl::memory::desc activation_desc{
            {rows, inner},
            activation_type,
            dnnl::memory::format_tag::ab
        };
        const dnnl::memory::desc weight_desc{
            {inner, columns},
            dnnl::memory::data_type::u4,
            dnnl::memory::format_tag::ab
        };
        const dnnl::memory::desc output_desc{
            {rows, columns},
            activation_type,
            dnnl::memory::format_tag::ab
        };
        const dnnl::memory::desc scale_desc{
            {groups, columns},
            activation_type,
            dnnl::memory::format_tag::ab
        };
        const dnnl::memory::desc zero_point_desc{
            {groups, columns},
            dnnl::memory::data_type::s8,
            dnnl::memory::format_tag::ab
        };
        const auto bias_desc =
            bias ? dnnl::memory::desc{{1, columns}, activation_type, dnnl::memory::format_tag::ab}
                 : dnnl::memory::desc{};

        auto attributes = SyclPrimitiveExecutor::user_scratchpad_attributes();
        attributes.set_scales(DNNL_ARG_WEIGHTS, 0b11, {m_group_size, 1}, activation_type);
        if (zero_point) {
            attributes.set_zero_points(
                DNNL_ARG_WEIGHTS,
                0b11,
                {m_group_size, 1},
                dnnl::memory::data_type::s8
            );
        }
        attributes.set_fpmath_mode(dnnl::fpmath_mode::any, true);

        SyclPrimitiveExecutor executor{queue->get()};
        const auto primitive_desc =
            bias
                ? dnnl::matmul::
                      primitive_desc{executor.getEngine(), activation_desc, weight_desc, bias_desc, output_desc, attributes}
                : dnnl::matmul::primitive_desc{
                      executor.getEngine(),
                      activation_desc,
                      weight_desc,
                      output_desc,
                      attributes
                  };
        executor.addArgument(DNNL_ARG_SRC, activation_desc, activation->get().getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, weight_desc, packed_weight->get().getData());
        executor.addArgument(DNNL_ARG_DST, output_desc, output->get().getData());
        executor.addArgument(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_WEIGHTS,
            scale_desc,
            scale->get().getData()
        );
        if (zero_point) {
            executor.addArgument(
                DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_WEIGHTS,
                zero_point_desc,
                zero_point->get().getData()
            );
        }
        if (bias) {
            executor.addArgument(DNNL_ARG_BIAS, bias_desc, bias->get().getData());
        }
        executor.execute(dnnl::matmul{primitive_desc}, primitive_desc);
    }

private:
    int64_t m_group_size;
    bool m_symmetric;
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
