module;

#include "include/c/intern/datatype.h"

#include <oneapi/dnnl/dnnl.hpp>

export module aten_xpu_extern_kernel:onednn_memory_layout;

import std;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclOnednnLayout
{
public:
    SyclOnednnLayout() = delete;

    static std::optional<dnnl::memory::data_type> data_type(TFDataTypeEnum dtype) noexcept
    {
        using data_type_t = dnnl::memory::data_type;
        switch (dtype) {
            case TF_FLOAT:
                return data_type_t::f32;
            case TF_HALF:
                return data_type_t::f16;
            case TF_BFLOAT16:
                return data_type_t::bf16;
            case TF_INT32:
            case TF_QINT32:
                return data_type_t::s32;
            case TF_INT8:
            case TF_QINT8:
                return data_type_t::s8;
            case TF_UINT8:
            case TF_QUINT8:
            case TF_BOOL:
                return data_type_t::u8;
            case TF_DOUBLE:
                return data_type_t::f64;
            case TF_FLOAT8_E4M3FN:
                return data_type_t::f8_e4m3;
            case TF_FLOAT8_E5M2:
                return data_type_t::f8_e5m2;
            default:
                return std::nullopt;
        }
    }

    static dnnl::memory::desc desc(const SyclTensor& tensor)
    {
        const auto type = data_type(tensor.getDtype()).value_or(dnnl::memory::data_type::f32);
        return dnnl::memory::desc{
            dnnl::memory::dims(tensor.getDims().begin(), tensor.getDims().end()),
            type,
            dnnl::memory::dims(tensor.getStrides().begin(), tensor.getStrides().end())
        };
    }

    static dnnl::memory::desc desc_as(const SyclTensor& tensor, dnnl::memory::data_type type)
    {
        return dnnl::memory::desc{
            dnnl::memory::dims(tensor.getDims().begin(), tensor.getDims().end()),
            type,
            dnnl::memory::dims(tensor.getStrides().begin(), tensor.getStrides().end())
        };
    }

    static dnnl::memory::desc any_desc(const dnnl::memory::desc& desc)
    {
        return dnnl::memory::desc{
            desc.get_dims(),
            desc.get_data_type(),
            dnnl::memory::format_tag::any
        };
    }

    static dnnl::memory::format_tag default_format(int rank, bool channels_last = false) noexcept
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

    static bool is_channels_last(const SyclTensor& tensor) noexcept
    {
        const auto& dims = tensor.getDims();
        const auto& strides = tensor.getStrides();
        if (dims.size() < 3) {
            return false;
        }
        return strides[1] == 1 && strides.back() == dims[1];
    }

    static bool is_matmul_compatible(const SyclTensor& tensor) noexcept
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

    static bool is_64_bytes_aligned(const SyclTensor& tensor) noexcept
    {
        return reinterpret_cast<std::uintptr_t>(tensor.getData()) % 64 == 0;
    }

    static dnnl::memory::desc bias_desc(const SyclTensor& bias, int spatial_rank)
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

    static dnnl::memory::dims
    padding_right(std::span<const int64_t> padding, std::span<const int64_t> output_padding)
    {
        dnnl::memory::dims result(padding.begin(), padding.end());
        for (std::size_t index = 0; index < output_padding.size() && index < result.size();
             ++index) {
            result[index] -= output_padding[index];
        }
        return result;
    }

    static dnnl::memory::dims dilation(std::span<const int64_t> dilation)
    {
        dnnl::memory::dims result(dilation.begin(), dilation.end());
        for (auto& value: result) {
            value -= 1;
        }
        return result;
    }
};

} // namespace aten_xpu
