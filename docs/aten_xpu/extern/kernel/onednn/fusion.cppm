module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_fusion;

import std;
import aten_xpu_intern;
import :onednn_post_op_attributes;

export namespace aten_xpu {

class SyclFusion
{
public:
    SyclFusion() = delete;

    static bool add_unary(
        SyclPostOpAttributes& attributes,
        std::string_view unary,
        std::span<const float> scalars,
        std::string_view algorithm
    )
    {
        if (unary == "none" || unary.empty()) {
            return true;
        }
        if (unary == "relu") {
            attributes.addEltwise(1.0F, 0.0F, 0.0F, dnnl::algorithm::eltwise_relu);
        } else if (unary == "sigmoid") {
            attributes.addEltwise(1.0F, 0.0F, 0.0F, dnnl::algorithm::eltwise_logistic);
        } else if (unary == "tanh") {
            attributes.addEltwise(1.0F, 0.0F, 0.0F, dnnl::algorithm::eltwise_tanh);
        } else if (unary == "hardswish") {
            attributes.addEltwise(1.0F, 1.0F / 6.0F, 0.5F, dnnl::algorithm::eltwise_hardswish);
        } else if (unary == "swish") {
            attributes.addEltwise(1.0F, 1.0F, 0.0F, dnnl::algorithm::eltwise_swish);
        } else if (unary == "hardsigmoid") {
            attributes.addEltwise(1.0F, 1.0F / 6.0F, 0.5F, dnnl::algorithm::eltwise_hardsigmoid);
        } else if (unary == "leaky_relu" && !scalars.empty()) {
            attributes.addEltwise(1.0F, scalars[0], 0.0F, dnnl::algorithm::eltwise_relu);
        } else if (unary == "hardtanh" && scalars.size() >= 2) {
            attributes.addEltwise(1.0F, scalars[0], scalars[1], dnnl::algorithm::eltwise_clip);
        } else if (unary == "gelu") {
            attributes.addEltwise(
                1.0F,
                0.0F,
                0.0F,
                algorithm == "tanh" ? dnnl::algorithm::eltwise_gelu_tanh
                                    : dnnl::algorithm::eltwise_gelu_erf
            );
        } else {
            return false;
        }
        return true;
    }

    static bool add_binary_then_unary(
        SyclPostOpAttributes& attributes,
        std::string_view binary,
        float input_scale,
        int64_t input_zero_point,
        std::optional<std::reference_wrapper<const SyclTensor>> accumulator,
        std::string_view unary,
        std::span<const float> scalars,
        std::string_view algorithm
    )
    {
        const bool unary_only = binary == "none";
        const bool valid_binary =
            (binary == "add" || binary == "sum") && (unary == "none" || unary == "relu");
        if (!unary_only && !valid_binary) {
            return false;
        }
        if (unary_only) {
            return add_unary(attributes, unary, scalars, algorithm);
        }

        if (binary == "sum") {
            if (input_zero_point != 0) {
                attributes.addEltwise(
                    1.0F,
                    1.0F,
                    -static_cast<float>(input_zero_point) * input_scale,
                    dnnl::algorithm::eltwise_linear
                );
            }
            attributes.addSum(1.0F, input_scale, 0);
        } else {
            if (!accumulator) {
                return false;
            }
            attributes.addBinary(dnnl::algorithm::binary_add, accumulator->get(), false);
        }
        if (unary == "relu") {
            attributes.addEltwise(1.0F, 0.0F, 0.0F, dnnl::algorithm::eltwise_relu);
        }
        return true;
    }

    static std::string_view activation_name(int64_t activation) noexcept
    {
        switch (activation) {
            case 1:
                return "relu";
            case 2:
                return "gelu";
            case 3:
                return "sigmoid";
            case 4:
                return "tanh";
            case 5:
                return "swish";
            case 6:
                return "hardswish";
            default:
                return "none";
        }
    }
};

} // namespace aten_xpu
