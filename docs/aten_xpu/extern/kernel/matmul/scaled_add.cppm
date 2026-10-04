module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:matmul_scaled_add;

import std;
import aten_xpu_intern;
import :onednn_post_op_attributes;
import :onednn_fusion;

export namespace aten_xpu {

class SyclScaledAdd
{
public:
    SyclScaledAdd() = delete;

    static SyclPostOpAttributes build(
        float alpha,
        float beta,
        std::optional<std::reference_wrapper<const SyclTensor>> addend,
        int64_t activation
    )
    {

        SyclPostOpAttributes attributes;
        const bool with_addend = addend.has_value() && beta != 0.0F;
        if (!with_addend) {
            if (alpha != 1.0F) {
                attributes.addEltwise(1.0F, alpha, 0.0F, dnnl::algorithm::eltwise_linear);
            }
        } else {
            if (alpha != beta) {
                attributes.addEltwise(1.0F, alpha / beta, 0.0F, dnnl::algorithm::eltwise_linear);
            }
            attributes.addBinary(dnnl::algorithm::binary_add, addend->get(), true);
            if (beta != 1.0F) {
                attributes.addEltwise(1.0F, beta, 0.0F, dnnl::algorithm::eltwise_linear);
            }
        }
        SyclFusion::add_unary(attributes, SyclFusion::activation_name(activation), {}, "none");
        return attributes;

    }
};

} // namespace aten_xpu
