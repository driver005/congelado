module;

#include "c/intern/tf_tensor.h"

export module cc_abi_builder_intern:tensor;

import std;
import cc_abi_primitives;
import :datatype;

export namespace ice::builder {

class Tensor
{
public:
    // Recover the Tensor instance from the opaque void* context slot that every
    // C vtable callback receives.  Named accessor so the cast intent is explicit
    // at the call site and the static_cast appears exactly once, here.
    static Tensor* create(void* ctx) noexcept
    {
        return static_cast<Tensor*>(ctx);
    }

    virtual ~Tensor() = default;

    virtual ice::String get_name() const noexcept = 0;

    // Range-first: the C ABI carries (const int64_t*, int) — the C++ interface takes a
    // span; the vtable lambda below is the only place the raw pair exists.
    [[nodiscard]] virtual std::expected<TF_Tensor_Handle*, ice::Status>
    allocate_tensor(ice::DataTypeEnum dtype, std::span<const int64_t> dims, size_t len) noexcept = 0;
    virtual void delete_tensor(TF_Tensor_Handle* tensor) noexcept = 0;
    virtual ice::DataTypeEnum get_type(const TF_Tensor_Handle* tensor) const noexcept = 0;
    virtual int get_num_dims(const TF_Tensor_Handle* tensor) const noexcept = 0;
    virtual int64_t get_dim(const TF_Tensor_Handle* tensor, int dim_index) const noexcept = 0;
    virtual int64_t get_num_elements(const TF_Tensor_Handle* tensor) const noexcept = 0;
    virtual size_t get_byte_size(const TF_Tensor_Handle* tensor) const noexcept = 0;
    virtual void* get_data(const TF_Tensor_Handle* tensor) const noexcept = 0;

    // Typed, range-first accessor — avoids static_cast<T*> at call sites that know the
    // element type and carries the element count.
    template<typename T>
    std::span<T> get_data_as(const TF_Tensor_Handle* tensor) const noexcept
    {
        return {static_cast<T*>(get_data(tensor)), static_cast<size_t>(get_num_elements(tensor))};
    }

    [[nodiscard]] virtual std::expected<void, ice::Status>
    bitcast_from(TF_Tensor_Handle* src, ice::DataTypeEnum dtype, TF_Tensor_Handle** out) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    bitcast_to(const TF_Tensor_Handle* src, ice::DataTypeEnum dtype, TF_Tensor_Handle** out) noexcept = 0;
    virtual void copy(TF_Tensor_Handle* src, TF_Tensor_Handle* dst) noexcept = 0;

    static TF_TensorOps* get_generic_vtable()
    {
        static TF_TensorOps vtable = {
            .struct_size = TF_TENSOR_STRUCT_SIZE,
            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                Tensor::create(plugin_context)->get_name().to_c(out);
            },
            .allocate_tensor = [](void* plugin_context,
                                  TF_DataType_Enum dtype,
                                  const int64_t* dims,
                                  int num_dims,
                                  size_t len) noexcept -> TF_Tensor_Handle*
            {
                // A negative num_dims (unknown rank) must not become a giant span.
                const std::span<const int64_t> dim_span =
                    num_dims > 0 ? std::span{dims, static_cast<size_t>(num_dims)}
                                 : std::span<const int64_t>{};
                auto res = Tensor::create(plugin_context)
                               ->allocate_tensor(ice::datatype_from_c(dtype), dim_span, len);
                if (!res) {
                    return nullptr; // no status slot — P1 allocator contract
                }
                return res.value();
            },
            .delete_tensor =
                [](void* plugin_context, TF_Tensor_Handle* tensor) noexcept
            {
                Tensor::create(plugin_context)->delete_tensor(tensor);
            },
            .tensor_type = [](void* plugin_context,
                              const TF_Tensor_Handle* tensor) noexcept -> TF_DataType_Enum
            {
                return datatype_to_c(Tensor::create(plugin_context)->get_type(tensor));
            },
            .num_dims = [](void* plugin_context, const TF_Tensor_Handle* tensor) noexcept -> int
            {
                return Tensor::create(plugin_context)->get_num_dims(tensor);
            },
            .dim =
                [](void* plugin_context, const TF_Tensor_Handle* tensor, int dim_index) noexcept
                -> int64_t
            {
                return Tensor::create(plugin_context)->get_dim(tensor, dim_index);
            },
            .tensor_element_count = [](void* plugin_context,
                                       const TF_Tensor_Handle* tensor) noexcept -> int64_t
            {
                return Tensor::create(plugin_context)->get_num_elements(tensor);
            },
            .tensor_byte_size = [](void* plugin_context, const TF_Tensor_Handle* tensor) noexcept
                                -> size_t
            {
                return Tensor::create(plugin_context)->get_byte_size(tensor);
            },
            .tensor_data = [](void* plugin_context, const TF_Tensor_Handle* tensor) noexcept
                           -> void*
            {
                return Tensor::create(plugin_context)->get_data(tensor);
            },
            .tensor_bitcast_from =
                [](void* plugin_context,
                   TF_Tensor_Handle* src,
                   TF_DataType_Enum dtype,
                   TF_Tensor_Handle** out,
                   TF_Status* status) noexcept
            {
                auto res = Tensor::create(plugin_context)
                               ->bitcast_from(src, ice::datatype_from_c(dtype), out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_bitcast_to =
                [](void* plugin_context,
                   const TF_Tensor_Handle* src,
                   TF_DataType_Enum dtype,
                   TF_Tensor_Handle** out,
                   TF_Status* status) noexcept
            {
                auto res = Tensor::create(plugin_context)
                               ->bitcast_to(src, ice::datatype_from_c(dtype), out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_copy =
                [](void* plugin_context, TF_Tensor_Handle* src, TF_Tensor_Handle* dst) noexcept
            {
                Tensor::create(plugin_context)->copy(src, dst);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
