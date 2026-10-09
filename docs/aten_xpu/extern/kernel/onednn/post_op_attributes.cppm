module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_post_op_attributes;

import std;
import aten_xpu_intern;
import :onednn_post_op;
import :onednn_memory_layout;

export namespace aten_xpu {

class SyclPostOpAttributes
{
public:
    SyclPostOpAttributes() = default;

    explicit SyclPostOpAttributes(float quantization_scale, int64_t zero_point = 0) noexcept :
        m_quantization_scale{quantization_scale},
        m_quantization_zero_point{zero_point}
    {
    }

    SyclPostOpAttributes&
    addSum(float scale, float sum_quantization_scale = 1.0F, int64_t zero_point = 0)
    {
        m_post_ops.push_back(SyclPostOp::sum(scale * sum_quantization_scale, zero_point));
        return *this;
    }

    SyclPostOpAttributes&
    addEltwise(float scale, float alpha, float beta, dnnl::algorithm algorithm)
    {
        m_post_ops.push_back(SyclPostOp::eltwise(scale, alpha, beta, algorithm));
        return *this;
    }

    SyclPostOpAttributes&
    addBinary(dnnl::algorithm algorithm, const SyclTensor& other, bool for_matmul)
    {
        const auto desc = SyclOnednnLayout::desc(other);
        m_post_ops.push_back(
            SyclPostOp::binary(
                algorithm,
                other.getData(),
                desc,
                for_matmul ? desc : SyclOnednnLayout::any_desc(desc)
            )
        );
        return *this;
    }

    SyclPostOpAttributes& addBias(const SyclTensor& bias, int spatial_rank)
    {
        const auto desc = SyclOnednnLayout::bias_desc(bias, spatial_rank);
        m_post_ops.push_back(
            SyclPostOp::binary(dnnl::algorithm::binary_add, bias.getData(), desc, desc)
        );
        return *this;
    }

    SyclPostOpAttributes& addPrelu(int mask)
    {
        m_post_ops.push_back(SyclPostOp::prelu(mask));
        return *this;
    }

    float getQuantizationScale() const noexcept
    {
        return m_quantization_scale;
    }

    int64_t getQuantizationZeroPoint() const noexcept
    {
        return m_quantization_zero_point;
    }

    bool empty() const noexcept
    {
        return m_post_ops.empty();
    }

    bool with_sum() const noexcept
    {
        return std::ranges::any_of(
            m_post_ops,
            [](const SyclPostOp& post_op)
            {
                return post_op.getKind() == dnnl::primitive::kind::sum;
            }
        );
    }

    bool with_binary() const noexcept
    {
        return std::ranges::any_of(
            m_post_ops,
            [](const SyclPostOp& post_op)
            {
                return post_op.getKind() == dnnl::primitive::kind::binary;
            }
        );
    }

    dnnl::post_ops extract() const
    {
        dnnl::post_ops result;
        for (const auto& post_op: m_post_ops) {
            switch (post_op.getKind()) {
                case dnnl::primitive::kind::eltwise:
                    result.append_eltwise(
                        post_op.getAlgorithm(),
                        post_op.getAlpha(),
                        post_op.getBeta()
                    );
                    break;
                case dnnl::primitive::kind::sum:
                    result.append_sum(
                        post_op.getScale(),
                        static_cast<int32_t>(post_op.getZeroPoint())
                    );
                    break;
                case dnnl::primitive::kind::binary:
                    result.append_binary(post_op.getAlgorithm(), post_op.getExpectedDesc());
                    break;
                case dnnl::primitive::kind::prelu:
                    result.append_prelu(post_op.getMask());
                    break;
                default:
                    break;
            }
        }
        return result;
    }

    void apply(dnnl::primitive_attr& attributes) const
    {
        if (!m_post_ops.empty()) {
            attributes.set_post_ops(extract());
        }
    }

    void add_binary_arguments(
        const dnnl::engine& engine,
        std::unordered_map<int, dnnl::memory>& arguments
    ) const
    {
        for (std::size_t index = 0; index < m_post_ops.size(); ++index) {
            const auto& post_op = m_post_ops[index];
            if (post_op.getKind() != dnnl::primitive::kind::binary) {
                continue;
            }
            arguments.insert_or_assign(
                DNNL_ARG_ATTR_MULTIPLE_POST_OP(static_cast<int>(index)) | DNNL_ARG_SRC_1,
                dnnl::memory{post_op.getDesc(), engine, const_cast<void*>(post_op.getBinaryData())}
            );
        }
    }

private:
    float m_quantization_scale{1.0F};
    int64_t m_quantization_zero_point{0};
    std::vector<SyclPostOp> m_post_ops;
};

} // namespace aten_xpu
