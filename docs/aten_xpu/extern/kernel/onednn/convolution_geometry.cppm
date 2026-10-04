module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_convolution_geometry;

import std;

export namespace aten_xpu {

class SyclConvolutionGeometry
{
public:
    void setStride(std::span<const int64_t> stride) { m_stride.assign(stride.begin(), stride.end()); }

    void setPadding(std::span<const int64_t> padding)
    {

        m_padding_left.assign(padding.begin(), padding.end());
        m_padding_right.assign(padding.begin(), padding.end());

    }

    void setOutputPadding(std::span<const int64_t> output_padding)
    {

        m_output_padding.assign(output_padding.begin(), output_padding.end());

    }

    void setDilation(std::span<const int64_t> dilation)
    {

        m_dilation.assign(dilation.begin(), dilation.end());

    }

    void setGroups(int64_t groups) noexcept { m_groups = groups; }

    void setSpatialRank(std::size_t rank)
    {

        m_stride.resize(rank, m_stride.empty() ? 1 : m_stride.back());
        m_padding_left.resize(rank, m_padding_left.empty() ? 0 : m_padding_left.back());
        m_padding_right.resize(rank, m_padding_right.empty() ? 0 : m_padding_right.back());
        m_output_padding.resize(rank, 0);
        m_dilation.resize(rank, m_dilation.empty() ? 1 : m_dilation.back());

    }

    const dnnl::memory::dims& getStride() const noexcept { return m_stride; }

    const dnnl::memory::dims& getPaddingLeft() const noexcept { return m_padding_left; }

    const dnnl::memory::dims& getPaddingRight() const noexcept { return m_padding_right; }

    const dnnl::memory::dims& getOutputPadding() const noexcept { return m_output_padding; }

    const dnnl::memory::dims& getDilation() const noexcept { return m_dilation; }

    int64_t getGroups() const noexcept { return m_groups; }

    dnnl::memory::dims onednn_dilation() const
    {

        dnnl::memory::dims result = m_dilation;
        for (auto& value: result) {
            value -= 1;
        }
        return result;

    }

    dnnl::memory::dims transposed_padding_right() const
    {

        dnnl::memory::dims result = m_padding_right;
        for (std::size_t index = 0; index < result.size(); ++index) {
            result[index] -= m_output_padding[index];
        }
        return result;

    }

    dnnl::memory::dims output_dims(std::span<const int64_t> source, std::span<const int64_t> weight) const
    {

        dnnl::memory::dims result(source.size());
        result[0] = source[0];
        result[1] = weight[0];
        for (std::size_t index = 2; index < source.size(); ++index) {
            const auto spatial = index - 2;
            const auto kernel = m_dilation[spatial] * (weight[index] - 1) + 1;
            result[index] =
                (source[index] + m_padding_left[spatial] + m_padding_right[spatial] - kernel) /
                    m_stride[spatial] +
                1;
        }
        return result;

    }

    dnnl::memory::dims transposed_output_dims(std::span<const int64_t> source, std::span<const int64_t> weight) const
    {

        dnnl::memory::dims result(source.size());
        result[0] = source[0];
        result[1] = weight[1] * m_groups;
        for (std::size_t index = 2; index < source.size(); ++index) {
            const auto spatial = index - 2;
            result[index] = (source[index] - 1) * m_stride[spatial] - m_padding_left[spatial] -
                            m_padding_right[spatial] + m_dilation[spatial] * (weight[index] - 1) +
                            m_output_padding[spatial] + 1;
        }
        return result;

    }

private:
    dnnl::memory::dims m_stride{1};
    dnnl::memory::dims m_padding_left{0};
    dnnl::memory::dims m_padding_right{0};
    dnnl::memory::dims m_output_padding{0};
    dnnl::memory::dims m_dilation{1};
    int64_t m_groups{1};
};

} // namespace aten_xpu
