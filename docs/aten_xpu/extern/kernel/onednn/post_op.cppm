module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_post_op;

import std;

export namespace aten_xpu {

class SyclPostOp
{
public:
    explicit SyclPostOp(dnnl::primitive::kind kind) noexcept :
        m_kind{kind}
    {
    }

    void setScale(float scale) noexcept
    {
        m_scale = scale;
    }

    void setAlpha(float alpha) noexcept
    {
        m_alpha = alpha;
    }

    void setBeta(float beta) noexcept
    {
        m_beta = beta;
    }

    void setZeroPoint(int64_t zero_point) noexcept
    {
        m_zero_point = zero_point;
    }

    void setMask(int mask) noexcept
    {
        m_mask = mask;
    }

    void setAlgorithm(dnnl::algorithm algorithm) noexcept
    {
        m_algorithm = algorithm;
    }

    void setBinaryData(const void* data) noexcept
    {
        m_binary_data = data;
    }

    void setDesc(const dnnl::memory::desc& desc)
    {
        m_desc = desc;
    }

    void setExpectedDesc(const dnnl::memory::desc& expected)
    {
        m_expected_desc = expected;
    }

    dnnl::primitive::kind getKind() const noexcept
    {
        return m_kind;
    }

    float getScale() const noexcept
    {
        return m_scale;
    }

    float getAlpha() const noexcept
    {
        return m_alpha;
    }

    float getBeta() const noexcept
    {
        return m_beta;
    }

    int64_t getZeroPoint() const noexcept
    {
        return m_zero_point;
    }

    int getMask() const noexcept
    {
        return m_mask;
    }

    dnnl::algorithm getAlgorithm() const noexcept
    {
        return m_algorithm;
    }

    const void* getBinaryData() const noexcept
    {
        return m_binary_data;
    }

    const dnnl::memory::desc& getDesc() const noexcept
    {
        return m_desc;
    }

    const dnnl::memory::desc& getExpectedDesc() const noexcept
    {
        return m_expected_desc;
    }

private:
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
