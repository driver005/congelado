#pragma once

// ice: replaced ATen/Context.h, ATen/native/transformers/attention.h,
//      ATen/native/transformers/sdp_utils_cpp.h,
//      ATen/native/transformers/xpu/flash_attn/utils.h,
//      ATen/xpu/XPUContext.h
//      with sycl/sycl.hpp and ice C-ABI headers.

#include "include/c/intern/datatype.h" // TFDataTypeEnum
#include "include/c/intern/tensor.h"   // TF_Tensor

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <sycl/sycl.hpp>

// ---------------------------------------------------------------------------
// SyclTensor — lightweight view of a device tensor passed across the ice ABI.
//   'data'  : USM device pointer (sycl::malloc_device / sycl::malloc_shared)
//   'dims'  : pointer to an array of 'ndim' extents (caller-owned lifetime)
//   'ndim'  : number of dimensions
//   'dtype' : element type (TFDataTypeEnum)
//   'queue' : SYCL queue on which the memory is live
// ---------------------------------------------------------------------------
struct SyclTensor
{
    void* data;
    const int64_t* dims; // dims[0..ndim-1]
    int ndim;
    TFDataTypeEnum dtype;
    sycl::queue* queue;

    // Convenience: return size of dimension d (supports negative indexing).
    [[nodiscard]] int64_t size(int d) const noexcept
    {
        if (d < 0) {
            d += ndim;
        }
        assert(d >= 0 && d < ndim);
        return dims[d];
    }

    // Returns true when the struct has been populated (data != nullptr).
    [[nodiscard]] bool defined() const noexcept
    {
        return data != nullptr;
    }
};

// ---------------------------------------------------------------------------
// SDP (Scaled Dot-Product Attention) parameter bag.
// ice: replaces sdp_params from ATen/native/transformers/sdp_utils_cpp.h.
//      at::Tensor  → SyclTensor
//      c10::SymInt → int64_t
//      c10::optional<T> is replaced by std::optional<T> throughout callers.
// ---------------------------------------------------------------------------
struct sdp_params
{
    SyclTensor query;
    SyclTensor key;
    SyclTensor value;
    bool is_causal{false};
    double dropout_p{0.0};
    // ice: std::optional<at::Tensor> attn_mask → std::optional<SyclTensor>
    std::optional<SyclTensor> attn_mask;
};

// ---------------------------------------------------------------------------
// XPU flash-attention availability / capability checks.
// ice: C10_EXPORT removed — symbol visibility controlled by the Bazel build.
// ---------------------------------------------------------------------------
namespace sdp {

[[nodiscard]] bool is_flash_attention_available();

[[nodiscard]] bool can_use_flash_attention(const sdp_params& params, bool debug);

[[nodiscard]] bool check_flash_attention_hardware_support(const sdp_params& params, bool debug);

} // namespace sdp
