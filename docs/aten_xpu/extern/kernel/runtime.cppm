module;

export module aten_xpu_extern_kernel:runtime;

import std;
import aten_xpu_intern;
import :onednn_runtime;
import :onednn_matmul_primitive;
import :onednn_convolution_primitive;
import :attention_onednn_sdpa;

export namespace aten_xpu {

class SyclKernelRuntime : public SyclSharedState
{
public:
    SyclKernelRuntime() :
        m_matmul{m_onednn},
        m_convolution{m_onednn},
        m_sdpa{m_onednn}
    {
    }

    SyclOnednnRuntime& getOnednn() noexcept
    {
        return m_onednn;
    }

    SyclMatmulPrimitive& getMatmul() noexcept
    {
        return m_matmul;
    }

    SyclConvolutionPrimitive& getConvolution() noexcept
    {
        return m_convolution;
    }

    SyclOnednnSdpa& getSdpa() noexcept
    {
        return m_sdpa;
    }

private:
    SyclOnednnRuntime m_onednn;
    SyclMatmulPrimitive m_matmul;
    SyclConvolutionPrimitive m_convolution;
    SyclOnednnSdpa m_sdpa;
};

} // namespace aten_xpu
