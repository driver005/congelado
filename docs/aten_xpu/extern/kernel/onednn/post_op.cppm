module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_post_op;

import std;

export namespace aten_xpu {

class SyclPostOp
{
public:
    static SyclPostOp eltwise(float scale, float alpha, float beta, dnnl::algorithm algorithm)
    {

        SyclPostOp post_op{dnnl::primitive::kind::eltwise};
        post_op.m_scale = scale;
        post_op.m_alpha = alpha;
        post_op.m_beta = beta;
        post_op.m_algorithm = algorithm;
        return post_op;

    }

    static SyclPostOp sum(float scale, int64_t zero_point)
    {

        SyclPostOp post_op{dnnl::primitive::kind::sum};
        post_op.m_scale = scale;
        post_op.m_zero_point = zero_point;
        return post_op;

    }

    static SyclPostOp binary(
        dnnl::algorithm algorithm,
        const void* data,
        const dnnl::memory::desc& desc,
        const dnnl::memory::desc& expected
    )
    {

        SyclPostOp post_op{dnnl::primitive::kind::binary};
        post_op.m_algorithm = algorithm;
        post_op.m_binary_data = data;
        post_op.m_desc = desc;
        post_op.m_expected_desc = expected;
        return post_op;

    }

    static SyclPostOp prelu(int mask)
    {

        SyclPostOp post_op{dnnl::primitive::kind::prelu};
        post_op.m_mask = mask;
        return post_op;

    }

    dnnl::primitive::kind getKind() const noexcept { return m_kind; }

    float getScale() const noexcept { return m_scale; }

    float getAlpha() const noexcept { return m_alpha; }

    float getBeta() const noexcept { return m_beta; }

    int64_t getZeroPoint() const noexcept { return m_zero_point; }

    int getMask() const noexcept { return m_mask; }

    dnnl::algorithm getAlgorithm() const noexcept { return m_algorithm; }

    const void* getBinaryData() const noexcept { return m_binary_data; }

    const dnnl::memory::desc& getDesc() const noexcept { return m_desc; }

    const dnnl::memory::desc& getExpectedDesc() const noexcept { return m_expected_desc; }

private:
    explicit SyclPostOp(dnnl::primitive::kind kind) noexcept :
        m_kind{kind}
    {
    }

    dnnl::primitive::kind m_kind;
    float m_scale{1.0F};
    float m_alpha{0.0F};
    float m_beta{0.0F};
    int64_t m_zero_point{0};
    int m_mask{0};
    dnnl::algorithm m_algorithm{dnnl::algorithm::eltwise_relu};
    const void* m_binary_data{nullptr};
    dnnl::memory::desc m_desc;
    dnnl::memory::desc m_expected_desc;
};

} // namespace aten_xpu
