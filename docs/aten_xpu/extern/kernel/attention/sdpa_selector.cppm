module;

#include "include/c/intern/datatype.h"

export module aten_xpu_extern_kernel:attention_sdpa_selector;

import std;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclSdpaSelector
{
public:
    enum class Backend : std::uint8_t
    {
        fused,
        math,
        invalid
    };

    static constexpr int64_t k_max_head_dim = 256;

    SyclSdpaSelector() = delete;

    static Backend choose(
        const SyclTensor& query,
        const SyclTensor& key,
        const SyclTensor& value,
        std::optional<std::reference_wrapper<const SyclTensor>> mask,
        float dropout,
        bool is_causal
    )
    {

        if (!valid_shapes(query, key, value)) {
            return Backend::invalid;
        }
        if (dropout > 0.0F || (mask && is_causal)) {
            return Backend::math;
        }
        if (!supported_dtype(query.getDtype()) || query.getDtype() != key.getDtype() ||
            query.getDtype() != value.getDtype())
        {
            return Backend::math;
        }
        const auto head_dim = query.getDims()[3];
        if (head_dim != value.getDims()[3] || head_dim > k_max_head_dim) {
            return Backend::math;
        }
        if (is_causal) {
            return Backend::math;
        }
        if (!query.is_contiguous() || !key.is_contiguous() || !value.is_contiguous()) {
            return Backend::math;
        }
        return Backend::fused;

    }

    static bool valid_shapes(const SyclTensor& query, const SyclTensor& key, const SyclTensor& value) noexcept
    {

        if (query.getDims().size() != 4 || key.getDims().size() != 4 || value.getDims().size() != 4) {
            return false;
        }
        const auto& query_dims = query.getDims();
        const auto& key_dims = key.getDims();
        const auto& value_dims = value.getDims();
        const bool batch_matches = query_dims[0] == key_dims[0] && query_dims[0] == value_dims[0];
        const bool heads_match = key_dims[1] == value_dims[1] && key_dims[1] > 0 && query_dims[1] % key_dims[1] == 0;
        const bool lengths_match = key_dims[2] == value_dims[2] && query_dims[3] == key_dims[3];
        return batch_matches && heads_match && lengths_match;

    }

private:
    static bool supported_dtype(TFDataTypeEnum dtype) noexcept
    {

        return dtype == TF_HALF || dtype == TF_BFLOAT16 || dtype == TF_FLOAT;

    }
};

} // namespace aten_xpu
