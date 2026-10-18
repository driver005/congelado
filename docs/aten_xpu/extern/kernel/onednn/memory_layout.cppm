module;

#include "include/c/intern/datatype.h"

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_memory_layout;

import std;
import cc_ice_intern_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclOnednnLayout
{
public:
    std::optional<dnnl::memory::data_type> data_type(ice::sonic::DataTypeEnum dtype) const noexcept
    {
        using data_type_t = dnnl::memory::data_type;
        switch (dtype) {
            case ice::sonic::DataTypeEnum::k_float:
                return data_type_t::f32;
            case ice::sonic::DataTypeEnum::k_half:
                return data_type_t::f16;
            case ice::sonic::DataTypeEnum::k_bfloat16:
                return data_type_t::bf16;
            case ice::sonic::DataTypeEnum::k_int32:
            case ice::sonic::DataTypeEnum::k_qint32:
                return data_type_t::s32;
            case ice::sonic::DataTypeEnum::k_int8:
            case ice::sonic::DataTypeEnum::k_qint8:
                return data_type_t::s8;
            case ice::sonic::DataTypeEnum::k_uint8:
            case ice::sonic::DataTypeEnum::k_quint8:
            case ice::sonic::DataTypeEnum::k_bool:
                return data_type_t::u8;
            case ice::sonic::DataTypeEnum::k_double:
                return data_type_t::f64;
            case ice::sonic::DataTypeEnum::k_float8_e4m3fn:
                return data_type_t::f8_e4m3;
            case ice::sonic::DataTypeEnum::k_float8_e5m2:
                return data_type_t::f8_e5m2;
            default:
                return std::nullopt;
        }
    }

    dnnl::memory::desc desc(const SyclTensor& tensor) const
    {
        const auto type = data_type(tensor.getDtype()).value_or(dnnl::memory::data_type::f32);
        return dnnl::memory::desc{
            dnnl::memory::dims(tensor.getDims().begin(), tensor.getDims().end()),
            type,
            dnnl::memory::dims(tensor.getStrides().begin(), tensor.getStrides().end())
        };
    }

    dnnl::memory::desc desc_as(const SyclTensor& tensor, dnnl::memory::data_type type) const
    {
        return dnnl::memory::desc{
            dnnl::memory::dims(tensor.getDims().begin(), tensor.getDims().end()),
            type,
            dnnl::memory::dims(tensor.getStrides().begin(), tensor.getStrides().end())
        };
    }

    dnnl::memory::desc any_desc(const dnnl::memory::desc& desc) const
    {
        return dnnl::memory::desc{
            desc.get_dims(),
            desc.get_data_type(),
            dnnl::memory::format_tag::any
        };
    }

    dnnl::memory::format_tag default_format(int rank, bool channels_last = false) const noexcept
    {
        using tag = dnnl::memory::format_tag;
        switch (rank) {
            case 1:
                return tag::a;
            case 2:
                return tag::ab;
            case 3:
                return channels_last ? tag::acb : tag::abc;
            case 4:
                return channels_last ? tag::acdb : tag::abcd;
            case 5:
                return channels_last ? tag::acdeb : tag::abcde;
            case 6:
                return tag::abcdef;
            default:
                return tag::undef;
        }
    }

    bool is_channels_last(const SyclTensor& tensor) const noexcept
    {
        const auto& dims = tensor.getDims();
        const auto& strides = tensor.getStrides();
        if (dims.size() < 3) {
            return false;
        }
        return strides[1] == 1 && strides.back() == dims[1];
    }

    bool is_matmul_compatible(const SyclTensor& tensor) const noexcept
    {
        const auto& dims = tensor.getDims();
        const auto& strides = tensor.getStrides();
        const auto rank = dims.size();
        if (rank < 2 || rank > 3) {
            return false;
        }
        if (tensor.element_count() == 0) {
            return true;
        }
        const bool row_major = strides[rank - 1] == 1 && strides[rank - 2] >= dims[rank - 1];
        const bool column_major = strides[rank - 2] == 1 && strides[rank - 1] >= dims[rank - 2];
        return row_major || column_major;
    }

    bool is_64_bytes_aligned(const SyclTensor& tensor) const noexcept
    {
        return reinterpret_cast<std::uintptr_t>(tensor.getData()) % 64 == 0;
    }

    dnnl::memory::desc bias_desc(const SyclTensor& bias, int spatial_rank) const
    {
        const auto channels = bias.getDims().empty() ? int64_t{1} : bias.getDims().front();
        dnnl::memory::dims dims{1, channels};
        dims.resize(static_cast<std::size_t>(spatial_rank) + 2, 1);
        return dnnl::memory::desc{
            dims,
            dnnl::memory::data_type::f32,
            default_format(static_cast<int>(dims.size()))
        };
    }

    dnnl::memory::dims
    padding_right(std::span<const int64_t> padding, std::span<const int64_t> output_padding) const
    {
        dnnl::memory::dims result(padding.begin(), padding.end());
        for (std::size_t index = 0; index < output_padding.size() && index < result.size();
             ++index) {
            result[index] -= output_padding[index];
        }
        return result;
    }

    dnnl::memory::dims dilation(std::span<const int64_t> dilation) const
    {
        dnnl::memory::dims result(dilation.begin(), dilation.end());
        for (auto& value: result) {
            value -= 1;
        }
        return result;
    }
};

} // namespace aten_xpu
