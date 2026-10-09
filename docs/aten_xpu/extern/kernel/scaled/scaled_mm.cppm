module;

#include "include/c/intern/datatype.h"

#include <oneapi/dnnl/dnnl.hpp>
#include <sycl/sycl.hpp>

export module aten_xpu_extern_kernel:scaled_scaled_mm;

import std;
import aten_xpu_intern;
import :kernel;
import :context;
import :construction;
import :onednn_memory_layout;
import :onednn_primitive_executor;
import :scaled_scaling_recipe;

export namespace aten_xpu {

class SyclScaledMatmulKernel : public SyclKernel<SyclScaledMatmulKernel>
{
public:
    static constexpr std::string_view k_name = "ScaledMatMul";

    SyclScaledMatmulKernel(const SyclOpsTable& ops, SyclKernelConstruction& construction) :
        SyclKernel<SyclScaledMatmulKernel>{ops},
        m_output_type{construction.getType("output_dtype", TF_BFLOAT16)},
        m_use_fast_accumulation{construction.getBool("use_fast_accum", false)}
    {
    }

    void compute(SyclKernelContext& context)
    {
        auto left = context.getInput(0);
        auto right = context.getInput(1);
        auto left_scale = context.getInput(2);
        auto right_scale = context.getInput(3);
        auto queue = context.getQueue();
        if (!left || !right || !left_scale || !right_scale || !queue) {
            context.propagate();
            return;
        }
        auto bias = context.getInputCount() > 4 ? context.getInput(4) : std::nullopt;

        const int64_t rows = left->get().getDims()[0];
        const int64_t inner = left->get().getDims()[1];
        const int64_t columns = right->get().getDims()[1];
        const auto left_kind = SyclScalingRecipe::detect(left_scale->get(), rows, inner, true);
        const auto right_kind =
            SyclScalingRecipe::detect(right_scale->get(), inner, columns, false);
        if (!SyclScalingRecipe::compatible(left_kind, right_kind)) {
            context.fail(TF_INVALID_ARGUMENT, "unsupported scaled_mm scaling recipe combination");
            return;
        }

        m_scratch_dims.assign({rows, columns});
        auto output = context.allocateOutput(0, m_output_type, m_scratch_dims);
        if (!output) {
            context.propagate();
            return;
        }

        const auto left_desc = SyclOnednnLayout::desc(left->get());
        const auto right_desc = SyclOnednnLayout::desc(right->get());
        const auto output_desc = SyclOnednnLayout::desc(output->get());
        const auto bias_desc =
            bias
                ? dnnl::memory::
                      desc{{1, columns}, SyclOnednnLayout::data_type(bias->get().getDtype()).value(), dnnl::memory::format_tag::ab}
                : dnnl::memory::desc{};

        auto attributes = SyclPrimitiveExecutor::user_scratchpad_attributes();
        SyclScalingRecipe::apply(attributes, DNNL_ARG_SRC, left_kind, true);
        SyclScalingRecipe::apply(attributes, DNNL_ARG_WEIGHTS, right_kind, false);
        if (m_use_fast_accumulation) {
            attributes.set_accumulation_mode(dnnl::accumulation_mode::relaxed);
        }

        SyclPrimitiveExecutor executor{queue->get()};
        const auto primitive_desc =
            bias
                ? dnnl::matmul::
                      primitive_desc{executor.getEngine(), left_desc, right_desc, bias_desc, output_desc, attributes}
                : dnnl::matmul::primitive_desc{
                      executor.getEngine(),
                      left_desc,
                      right_desc,
                      output_desc,
                      attributes
                  };
        executor.addArgument(DNNL_ARG_SRC, left_desc, left->get().getData());
        executor.addArgument(DNNL_ARG_WEIGHTS, right_desc, right->get().getData());
        executor.addArgument(DNNL_ARG_DST, output_desc, output->get().getData());
        executor.addArgument(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_SRC,
            SyclOnednnLayout::desc(left_scale->get()),
            left_scale->get().getData()
        );
        executor.addArgument(
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_WEIGHTS,
            SyclOnednnLayout::desc(right_scale->get()),
            right_scale->get().getData()
        );
        if (bias) {
            executor.addArgument(DNNL_ARG_BIAS, bias_desc, bias->get().getData());
        }
        executor.execute(dnnl::matmul{primitive_desc}, primitive_desc);
    }

private:
    TFDataTypeEnum m_output_type;
    bool m_use_fast_accumulation;
    std::vector<int64_t> m_scratch_dims;
};

} // namespace aten_xpu
