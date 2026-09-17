// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tensor.h"

export module cc_abi_builder_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_TensorOps
{
public:
    static TF_TensorOps* create(void* ctx) noexcept
    {
        return static_cast<TF_TensorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TensorOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_TensorOps*>(handle->plugin_data);
    }

    virtual ~TF_TensorOps() = default;
    [[nodiscard]] std::expected<void, ice::Status> set_dtype(TFDataTypeEnum dtype) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_dims(const int64_t* dims, int num_dims) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_byte_size(size_t len) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_tensor() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_type(TFDataTypeEnum* out_dtype) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> num_dims(int* out_num_dims) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    dim(int dim_index, int64_t* out_dim) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_element_count(int64_t* out_count) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_byte_size(size_t* out_byte_size) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> tensor_data(void** out_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_from(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_to(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_copy(const ice::sonic::TF_TensorOps& dst) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_TensorOps* get_generic_vtable()
    {
        static TF_TensorOps vtable = {
            .struct_size = TF_TENSOR_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_TensorOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .set_dtype =
                [](TF_Tensor* tensor, TFDataTypeEnum dtype) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->set_dtype(dtype);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_dims =
                [](TF_Tensor* tensor, const int64_t* dims, int num_dims) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->set_dims(dims, num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_byte_size =
                [](TF_Tensor* tensor, size_t len) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->set_byte_size(len);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_tensor =
                [](TF_Tensor* tensor) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->delete_tensor();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_type =
                [](const TF_Tensor* tensor, TFDataTypeEnum* out_dtype) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->tensor_type(out_dtype);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .num_dims =
                [](const TF_Tensor* tensor, int* out_num_dims) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->num_dims(out_num_dims);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .dim =
                [](const TF_Tensor* tensor, int dim_index, int64_t* out_dim) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->dim(dim_index, out_dim);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_element_count =
                [](const TF_Tensor* tensor, int64_t* out_count) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->tensor_element_count(out_count);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_byte_size =
                [](const TF_Tensor* tensor, size_t* out_byte_size) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->tensor_byte_size(out_byte_size);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_data =
                [](const TF_Tensor* tensor, void** out_data) noexcept
            {
                auto* self = TF_TensorOps::create(tensor);
                auto res = self->tensor_data(out_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_bitcast_from =
                [](TF_Tensor* src,
                   TFDataTypeEnum dtype,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_TensorOps::create(src);
                auto res = self->tensor_bitcast_from(dtype, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .tensor_bitcast_to =
                [](const TF_Tensor* src,
                   TFDataTypeEnum dtype,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto* self = TF_TensorOps::create(src);
                auto res = self->tensor_bitcast_to(dtype, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .tensor_copy =
                [](TF_Tensor* src, TF_Tensor* dst) noexcept
            {
                auto* self = TF_TensorOps::create(src);
                auto res = self->tensor_copy(ice::sonic::TF_TensorOps::wrap(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
