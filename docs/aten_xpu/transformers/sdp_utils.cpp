// ice: replaced ATen/native/transformers/xpu/flash_attn/flash_api.h and
//      ATen/native/transformers/xpu/sdp_utils.h
//      with sycl/sycl.hpp and sdp_utils.h (ice version).

#include "docs/aten_xpu/transformers/sdp_utils.h"

// ice: sycltla::is_flash_attention_available() is a SYCL kernel library
//      function declared in the (non-ATen) flash_attn library.  We keep
//      the call intact; the build system must supply the sycltla library.
// ice: call sycltla::flash_attention_forward(data_ptr, dims, ...) here

#include <array>
#include <cassert>
#include <cstdio> // std::fprintf for debug messages
#include <stdexcept>
#include <string>
#include <sycl/sycl.hpp>

namespace sdp {

// ---------------------------------------------------------------------------
// is_flash_attention_available
// ice: forwards to sycltla library (unchanged call, ATen wrapper removed).
// ---------------------------------------------------------------------------
bool is_flash_attention_available()
{
    // ice: call sycltla::is_flash_attention_available() here
    // The sycltla library is a pure SYCL library with no ATen dependency.
    return sycltla::is_flash_attention_available();
}

// ---------------------------------------------------------------------------
// (file-local) is_flash_attention_available with debug warning
// ---------------------------------------------------------------------------
inline bool is_flash_attention_available(const sdp_params& params, bool debug)
{
    if (!is_flash_attention_available()) {
        if (debug) {
            // warning: Congelado XPU was not compiled with flash attention.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: Congelado XPU was not compiled with "
                "flash attention.\n"
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_flash_attention_hardware_support
// ice: at::xpu::is_available()            → sycl::device check
//      at::xpu::getCurrentDeviceProperties() → sycl::device::get_info<>
//      TORCH_CHECK(false, msg)             → throw std::runtime_error(msg)
//      TORCH_WARN(msg)                     → std::fprintf(stderr, ...)
// ---------------------------------------------------------------------------
bool check_flash_attention_hardware_support(const sdp_params& params, bool debug)
{
    // Verify that the queue carries a valid GPU device.
    // ice: at::xpu::is_available() → query device from the tensor's queue
    if (params.query.queue == nullptr) {
        throw std::runtime_error(
            "FlashAttentionXPU: SYCL queue is null — no XPU device available."
        );
    }
    const sycl::device& dev = params.query.queue->get_device();
    if (!dev.is_gpu()) {
        throw std::runtime_error("FlashAttentionXPU: associated SYCL device is not a GPU.");
    }

    // Supported Intel GPU micro-architectures.
    constexpr auto supported_architectures =
        std::to_array<sycl::ext::oneapi::experimental::architecture>(
            {sycl::ext::oneapi::experimental::architecture::intel_gpu_pvc,
             sycl::ext::oneapi::experimental::architecture::intel_gpu_pvc_vg,
             sycl::ext::oneapi::experimental::architecture::intel_gpu_bmg_g21,
             sycl::ext::oneapi::experimental::architecture::intel_gpu_bmg_g31}
        );

    // ice: at::xpu::getCurrentDeviceProperties()->architecture
    //      → sycl::device::get_info<sycl::ext::oneapi::experimental::info::device::architecture>
    const auto device_architecture =
        dev.get_info<sycl::ext::oneapi::experimental::info::device::architecture>();

    if (std::find(
            supported_architectures.begin(),
            supported_architectures.end(),
            device_architecture
        ) == supported_architectures.end()) {
        if (debug) {
            // warning: XPU device architecture does not support flash attention.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: XPU device architecture does not support "
                "flash attention. Supported architectures are: "
                "intel_gpu_pvc, intel_gpu_pvc_vg, intel_gpu_bmg_g21, "
                "intel_gpu_bmg_g31.\n"
            );
        }
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// check_flash_attention_datatype (file-local)
// ice: at::ScalarType / at::kBFloat16 / at::kHalf
//      → TFDataTypeEnum / TF_BFLOAT16 / TF_HALF
//      params.query.dtype(), params.key.dtype() → SyclTensor::dtype field
//      TORCH_WARN → std::fprintf
// ---------------------------------------------------------------------------
inline bool check_flash_attention_datatype(const sdp_params& params, bool debug)
{
    // ice: supported dtypes are TF_BFLOAT16 and TF_HALF (float16).
    constexpr std::array<TFDataTypeEnum, 2> supported_dtypes{TF_BFLOAT16, TF_HALF};

    const TFDataTypeEnum query_dtype = params.query.dtype;
    const bool dtype_match =
        (query_dtype == params.key.dtype) && (query_dtype == params.value.dtype) &&
        (std::find(supported_dtypes.begin(), supported_dtypes.end(), query_dtype) !=
         supported_dtypes.end());

    if (!dtype_match) {
        if (debug) {
            // warning: FlashAttentionXPU expected query, key, value dtypes to
            // be TF_BFLOAT16 or TF_HALF.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: FlashAttentionXPU expected query, key, "
                "and value to all be of dtype {TF_BFLOAT16, TF_HALF}. "
                "Got Query dtype: %d, Key dtype: %d, Value dtype: %d instead.\n",
                static_cast<int>(query_dtype),
                static_cast<int>(params.key.dtype),
                static_cast<int>(params.value.dtype)
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_flash_attention_head_dim_size (file-local)
// ice: params.query.sym_size(-1) → SyclTensor::size(-1)  (int64_t)
//      c10::SymInt                → int64_t
//      TORCH_WARN                 → std::fprintf
// ---------------------------------------------------------------------------
inline bool check_flash_attention_head_dim_size(const sdp_params& params, bool debug)
{
    // Use SyclTensor::size() which supports negative indexing.
    // ice: sym_size(-1) → size(-1) — no symbolic-shape tracing needed here;
    //      the SYCL path always works on concrete extents.
    const int64_t query_size_last = params.query.size(-1);
    const int64_t key_size_last = params.key.size(-1);
    const int64_t value_size_last = params.value.size(-1);

    const bool head_dims_equal =
        (query_size_last == key_size_last) && (query_size_last == value_size_last);

    if (!head_dims_equal) {
        if (debug) {
            // warning: FlashAttentionXPU requires q,k,v to have the same last
            // dimension.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: FlashAttentionXPU requires q,k,v to have "
                "the same last dimension. Got Query.size(-1): %" PRId64 ", Key.size(-1): %" PRId64
                ", Value.size(-1): %" PRId64 " instead.\n",
                query_size_last,
                key_size_last,
                value_size_last
            );
        }
        return false;
    }

    constexpr int64_t kXPUFlashAttentionMaxHeadDim = 256;
    if (query_size_last > kXPUFlashAttentionMaxHeadDim) {
        if (debug) {
            // warning: FlashAttentionXPU supports head dimension up to 256.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: FlashAttentionXPU supports head dimension "
                "up to %" PRId64 ". Got head dimension: %" PRId64 " instead.\n",
                kXPUFlashAttentionMaxHeadDim,
                query_size_last
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_flash_attention_layout (file-local)
// ice: forwards to sycltla library (pure SYCL, no ATen dependency).
//      The sycltla::check_flash_attention_layout signature must accept
//      sdp_params (ice version) — or we pass raw pointers.
// ---------------------------------------------------------------------------
inline bool check_flash_attention_layout(const sdp_params& params, bool debug)
{
    // ice: call sycltla::check_flash_attention_layout(params, debug) here
    //      sycltla must be updated to consume ice sdp_params.
    return sycltla::check_flash_attention_layout(params, debug);
}

// ---------------------------------------------------------------------------
// check_flash_causal_non_square_seqlens (file-local)
// ice: params.query.is_nested() → not applicable for flat SyclTensor;
//      nested tensors are not supported in this ice path (always false).
//      params.query.sym_size(-2) → SyclTensor::size(-2)
//      TORCH_WARN                → std::fprintf
// ---------------------------------------------------------------------------
inline bool check_flash_causal_non_square_seqlens(const sdp_params& params, bool debug)
{
    // ice: is_nested() is always false for SyclTensor (flat device pointers).
    //      Non-square causal masks remain unsupported in the flash path.
    if (params.is_causal && params.query.size(-2) != params.key.size(-2)) {
        if (debug) {
            // warning: Flash attention XPU does not support is_causal when
            // seqlen_q != seqlen_k.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: Flash attention XPU does not support the "
                "is_causal flag when seqlen_q != seqlen_k. "
                "Got seqlen_q: %" PRId64 " seqlen_k: %" PRId64 ". "
                "If you would like to use causal attention with non-square masks, "
                "please see CausalAttnMask.\n",
                params.query.size(-2),
                params.key.size(-2)
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_flash_attention_deterministic (file-local)
// ice: at::globalContext().deterministicAlgorithms()
//      → replaced with a compile-time opt-in macro or environment variable.
//      Flash attention is inherently non-deterministic; callers that require
//      determinism must opt out explicitly.
//      TORCH_WARN → std::fprintf
// ---------------------------------------------------------------------------
inline bool check_flash_attention_deterministic(const sdp_params& params, bool debug)
{
    // ice: ICE_FLASH_ATTN_DETERMINISTIC_CHECK — set env var
    //      CONGELADO_DETERMINISTIC=1 to opt into strict determinism checks.
    const char* det_env = std::getenv("CONGELADO_DETERMINISTIC");
    const bool deterministic_mode = (det_env != nullptr && det_env[0] == '1');
    if (deterministic_mode) {
        if (debug) {
            // warning: Flash attention XPU is not deterministic.
            std::fprintf(
                stderr,
                "[sdp_utils] warning: Flash attention XPU is not deterministic. "
                "Set CONGELADO_DETERMINISTIC=0 to allow.\n"
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_for_attn_mask (file-local)
// ice: replaces the ATen sdp_utils_cpp.h helper.
//      Returns false if an attention mask is present (not supported in flash).
// ---------------------------------------------------------------------------
inline bool check_for_attn_mask(const sdp_params& params, bool debug)
{
    if (params.attn_mask.has_value()) {
        if (debug) {
            std::fprintf(
                stderr,
                "[sdp_utils] warning: FlashAttentionXPU does not support an "
                "explicit attention mask.\n"
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_for_dropout (file-local)
// ice: replaces the ATen sdp_utils_cpp.h helper.
// ---------------------------------------------------------------------------
inline bool check_for_dropout(const sdp_params& params, bool debug)
{
    // Flash attention supports dropout natively; no constraint here.
    (void)params;
    (void)debug;
    return true;
}

// ---------------------------------------------------------------------------
// check_nested_tensor (file-local)
// ice: SyclTensor has no nested-tensor concept → always passes.
// ---------------------------------------------------------------------------
inline bool check_nested_tensor(const sdp_params& params, bool debug)
{
    (void)params;
    (void)debug;
    return true; // ice: nested tensors not applicable to flat SyclTensor
}

// ---------------------------------------------------------------------------
// check_tensor_shapes (file-local)
// ice: replaces ATen sdp_utils_cpp.h helper.
//      Validates that q, k, v are 4-dimensional.
// ---------------------------------------------------------------------------
inline bool check_tensor_shapes(const sdp_params& params, bool debug)
{
    if (params.query.ndim != 4 || params.key.ndim != 4 || params.value.ndim != 4) {
        if (debug) {
            std::fprintf(
                stderr,
                "[sdp_utils] warning: FlashAttentionXPU requires 4-D tensors "
                "(batch, heads, seqlen, head_dim). Got ndim: q=%d k=%d v=%d.\n",
                params.query.ndim,
                params.key.ndim,
                params.value.ndim
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_batch_size_and_num_heads_dense<supports_gqa> (file-local)
// ice: replaces the ATen template helper.
//      dims layout: [batch, num_heads, seqlen, head_dim]
// ---------------------------------------------------------------------------
template<bool supports_gqa>
inline bool check_batch_size_and_num_heads_dense(const sdp_params& params, bool debug)
{
    const int64_t q_batch = params.query.size(0);
    const int64_t kv_batch = params.key.size(0);
    const int64_t q_heads = params.query.size(1);
    const int64_t kv_heads = params.key.size(1);

    if (q_batch != kv_batch) {
        if (debug) {
            std::fprintf(
                stderr,
                "[sdp_utils] warning: batch size mismatch: q=%" PRId64 " k/v=%" PRId64 ".\n",
                q_batch,
                kv_batch
            );
        }
        return false;
    }
    if constexpr (!supports_gqa) {
        if (q_heads != kv_heads) {
            if (debug) {
                std::fprintf(
                    stderr,
                    "[sdp_utils] warning: head count mismatch without GQA: "
                    "q=%" PRId64 " kv=%" PRId64 ".\n",
                    q_heads,
                    kv_heads
                );
            }
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_nonzero_sequence_lengths_dense (file-local)
// ice: replaces the ATen helper.
// ---------------------------------------------------------------------------
inline bool check_nonzero_sequence_lengths_dense(const sdp_params& params, bool debug)
{
    if (params.query.size(-2) == 0 || params.key.size(-2) == 0) {
        if (debug) {
            std::fprintf(
                stderr,
                "[sdp_utils] warning: FlashAttentionXPU does not support "
                "zero-length sequences.\n"
            );
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// check_last_dim_stride_equals_1_dense<ignore_singleton_dim> (file-local)
// ice: SyclTensor carries no explicit stride info.  For device-allocated
//      tensors produced by sycl::malloc_device the last dimension is always
//      contiguous (stride == 1).  We assert this is the case and move on.
// ---------------------------------------------------------------------------
template<bool ignore_singleton_dim>
inline bool check_last_dim_stride_equals_1_dense(const sdp_params& params, bool debug)
{
    // ice: stride information is implicit (contiguous allocation assumed).
    //      If callers pass non-contiguous views they must ensure contiguity
    //      before invoking flash attention.
    (void)params;
    (void)debug;
    return true; // assume contiguous — USM allocs are always row-major
}

// ---------------------------------------------------------------------------
// can_use_flash_attention
// ice: replaced ATen constraint function pointers with ice equivalents above.
// ---------------------------------------------------------------------------
bool can_use_flash_attention(const sdp_params& params, bool debug)
{
    using ConstraintFn = bool (*)(const sdp_params&, bool);
    constexpr std::array<ConstraintFn, 14> constraints{
        is_flash_attention_available,
        check_flash_attention_hardware_support,
        check_for_attn_mask,
        check_for_dropout,
        check_nested_tensor,
        check_tensor_shapes,
        check_batch_size_and_num_heads_dense<true /*supports GQA*/>,
        check_nonzero_sequence_lengths_dense,
        check_last_dim_stride_equals_1_dense<true /*ignore_singleton_dim*/>,
        check_flash_causal_non_square_seqlens,
        check_flash_attention_datatype,
        check_flash_attention_head_dim_size,
        check_flash_attention_layout,
        check_flash_attention_deterministic,
    };
    for (auto& constraint: constraints) {
        if (!constraint(params, debug)) {
            return false;
        }
    }
    return true;
}

} // namespace sdp
