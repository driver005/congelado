// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/tensor/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/tensor/tensor.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_tensor;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Tensor
{
public:
    static Tensor* create(void* ctx) noexcept
    {
        return static_cast<Tensor*>(ctx);
    }

    template<typename HandleT>
    static Tensor* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Tensor*>(handle);
    }

    virtual ~Tensor() = default;
    [[nodiscard]] std::expected<void, ice::Status> allocate_tensor(
        TF_DataType_Enum dtype,
        const int64_t* dims,
        int num_dims,
        size_t len
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> delete_tensor() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> tensor_type() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> num_dims() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> dim(int dim_index) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> tensor_element_count() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> tensor_byte_size() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> tensor_data() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_from(TF_DataType_Enum dtype, TF_Tensor_Handle** out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_bitcast_to(TF_DataType_Enum dtype, TF_Tensor_Handle** out) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    tensor_copy(const ice::sonic::Tensor& dst) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_Tensor* get_generic_vtable()
    {
        static TF_Tensor vtable = {
            .struct_size = TF_TENSOR_STRUCT_SIZE,

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Tensor::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .allocate_tensor =
                [](void* plugin_context,
                   TF_DataType_Enum dtype,
                   const int64_t* dims,
                   int num_dims,
                   size_t len) noexcept
            {
                auto* self = Tensor::create(plugin_context);
                auto res = self->allocate_tensor(dtype, dims, num_dims, len);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_tensor =
                [](TF_Tensor_Handle* tensor) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->delete_tensor();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_type =
                [](const TF_Tensor_Handle* tensor) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->tensor_type();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .num_dims =
                [](const TF_Tensor_Handle* tensor) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->num_dims();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .dim =
                [](const TF_Tensor_Handle* tensor, int dim_index) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->dim(dim_index);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_element_count =
                [](const TF_Tensor_Handle* tensor) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->tensor_element_count();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_byte_size =
                [](const TF_Tensor_Handle* tensor) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->tensor_byte_size();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_data =
                [](const TF_Tensor_Handle* tensor) noexcept
            {
                auto* self = Tensor::create(tensor);
                auto res = self->tensor_data();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_bitcast_from =
                [](TF_Tensor_Handle* src,
                   TF_DataType_Enum dtype,
                   TF_Tensor_Handle** out,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Tensor::create(src);
                auto res = self->tensor_bitcast_from(dtype, out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_bitcast_to =
                [](const TF_Tensor_Handle* src,
                   TF_DataType_Enum dtype,
                   TF_Tensor_Handle** out,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Tensor::create(src);
                auto res = self->tensor_bitcast_to(dtype, out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_copy =
                [](TF_Tensor_Handle* src, TF_Tensor_Handle* dst) noexcept
            {
                auto* self = Tensor::create(src);
                auto res = self->tensor_copy(ice::sonic::Tensor::wrap(dst));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
