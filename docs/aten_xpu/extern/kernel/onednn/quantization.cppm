module;

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_quantization;

import std;
import aten_xpu_intern;
import :onednn_memory_layout;
import :onednn_primitive_executor;

export namespace aten_xpu {

class SyclQuantization
{
public:
    void setSourceScale(const SyclTensor& scale)
    {
        m_source_scale = scale;
    }

    void setSourceZeroPoint(const SyclTensor& zero_point)
    {
        m_source_zero_point = zero_point;
    }

    void setWeightScale(const SyclTensor& scale, int per_channel_mask)
    {
        m_weight_scale = scale;
        m_weight_mask = scale.element_count() > 1 ? per_channel_mask : 0;
    }

    void setDestinationScale(const SyclTensor& scale)
    {
        m_destination_scale = scale;
    }

    void setDestinationZeroPoint(const SyclTensor& zero_point)
    {
        m_destination_zero_point = zero_point;
    }

    void apply(dnnl::primitive_attr& attributes) const
    {
        if (m_source_scale) {
            attributes.set_scales_mask(DNNL_ARG_SRC, 0);
        }
        if (m_source_zero_point) {
            attributes.set_zero_points_mask(DNNL_ARG_SRC, 0);
        }
        if (m_weight_scale) {
            attributes.set_scales_mask(DNNL_ARG_WEIGHTS, m_weight_mask);
        }
        if (m_destination_scale) {
            attributes.set_scales_mask(DNNL_ARG_DST, 0);
        }
        if (m_destination_zero_point) {
            attributes.set_zero_points_mask(DNNL_ARG_DST, 0);
        }
    }

    void add_arguments(SyclPrimitiveExecutor& executor) const
    {
        add(executor,
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_SRC,
            m_source_scale,
            dnnl::memory::data_type::f32);
        add(executor,
            DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_SRC,
            m_source_zero_point,
            dnnl::memory::data_type::s32);
        add(executor,
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_WEIGHTS,
            m_weight_scale,
            dnnl::memory::data_type::f32);
        add(executor,
            DNNL_ARG_ATTR_SCALES | DNNL_ARG_DST,
            m_destination_scale,
            dnnl::memory::data_type::f32);
        add(executor,
            DNNL_ARG_ATTR_ZERO_POINTS | DNNL_ARG_DST,
            m_destination_zero_point,
            dnnl::memory::data_type::s32);
    }

private:
    static void
    add(SyclPrimitiveExecutor& executor,
        int argument,
        const std::optional<std::reference_wrapper<const SyclTensor>>& tensor,
        dnnl::memory::data_type type)
    {
        if (!tensor) {
            return;
        }
        const dnnl::memory::desc desc{
            {tensor->get().element_count()},
            type,
            dnnl::memory::format_tag::x
        };
        executor.addArgument(argument, desc, tensor->get().getData());
    }

    std::optional<std::reference_wrapper<const SyclTensor>> m_source_scale;
    std::optional<std::reference_wrapper<const SyclTensor>> m_source_zero_point;
    std::optional<std::reference_wrapper<const SyclTensor>> m_weight_scale;
    std::optional<std::reference_wrapper<const SyclTensor>> m_destination_scale;
    std::optional<std::reference_wrapper<const SyclTensor>> m_destination_zero_point;
    int m_weight_mask{0};
};

} // namespace aten_xpu
