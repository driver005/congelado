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

    virtual ice::String get_name() const = 0;

    virtual [[nodiscard]] std::expected<TF_Tensor_Handle*, ice::Status>
    allocate_tensor(ice::DataTypeEnum dtype, const int64_t* dims, int num_dims, size_t len) = 0;
    virtual void delete_tensor(TF_Tensor_Handle* tensor) = 0;
    virtual ice::DataTypeEnum get_type(const TF_Tensor_Handle* tensor) const = 0;
    virtual int get_num_dims(const TF_Tensor_Handle* tensor) const = 0;
    virtual int64_t get_dim(const TF_Tensor_Handle* tensor, int dim_index) const = 0;
    virtual int64_t get_num_elements(const TF_Tensor_Handle* tensor) const = 0;
    virtual size_t get_byte_size(const TF_Tensor_Handle* tensor) const = 0;
    virtual void* get_data(const TF_Tensor_Handle* tensor) const = 0;

    // Typed accessor — avoids static_cast<T*> at call sites that know the element type.
    template<typename T>
    T* get_data_as(const TF_Tensor_Handle* tensor) const
    {
        return static_cast<T*>(get_data(tensor));
    }

    virtual [[nodiscard]] std::expected<void, ice::Status>
    bitcast_from(TF_Tensor_Handle* src, ice::DataTypeEnum dtype, TF_Tensor_Handle** out) = 0;
    virtual [[nodiscard]] std::expected<void, ice::Status>
    bitcast_to(const TF_Tensor_Handle* src, ice::DataTypeEnum dtype, TF_Tensor_Handle** out) = 0;
    virtual void copy(TF_Tensor_Handle* src, TF_Tensor_Handle* dst) = 0;

    static TF_TensorOps* get_generic_vtable()
    {
        static TF_TensorOps vtable = {
            .struct_size = sizeof(TF_TensorOps),
            .get_name =
                [](void* plugin_context, TF_String* out)
            {
                Tensor::create(plugin_context)->get_name().to_c(out);
            },
            .allocate_tensor = [](void* plugin_context,
                                  TF_DataType_Enum dtype,
                                  const int64_t* dims,
                                  int num_dims,
                                  size_t len) -> TF_Tensor_Handle*
            {
                auto res = Tensor::create(plugin_context)
                               ->allocate_tensor(ice::data_type_from_c(dtype), dims, num_dims, len);
                if (!res) {
                    return nullptr; // no status slot — P1 allocator contract
                }
                return res.value();
            },
            .delete_tensor =
                [](void* plugin_context, TF_Tensor_Handle* tensor)
            {
                Tensor::create(plugin_context)->delete_tensor(tensor);
            },
            .tensor_type = [](void* plugin_context,
                              const TF_Tensor_Handle* tensor) -> TF_DataType_Enum
            {
                return data_type_to_c(Tensor::create(plugin_context)->get_type(tensor));
            },
            .num_dims = [](void* plugin_context, const TF_Tensor_Handle* tensor) -> int
            {
                return Tensor::create(plugin_context)->get_num_dims(tensor);
            },
            .dim =
                [](void* plugin_context, const TF_Tensor_Handle* tensor, int dim_index) -> int64_t
            {
                return Tensor::create(plugin_context)->get_dim(tensor, dim_index);
            },
            .tensor_element_count = [](void* plugin_context,
                                       const TF_Tensor_Handle* tensor) -> int64_t
            {
                return Tensor::create(plugin_context)->get_num_elements(tensor);
            },
            .tensor_byte_size = [](void* plugin_context, const TF_Tensor_Handle* tensor) -> size_t
            {
                return Tensor::create(plugin_context)->get_byte_size(tensor);
            },
            .tensor_data = [](void* plugin_context, const TF_Tensor_Handle* tensor) -> void*
            {
                return Tensor::create(plugin_context)->get_data(tensor);
            },
            .tensor_bitcast_from =
                [](void* plugin_context,
                   TF_Tensor_Handle* src,
                   TF_DataType_Enum dtype,
                   TF_Tensor_Handle** out,
                   TF_Status* status)
            {
                auto res = Tensor::create(plugin_context)
                               ->bitcast_from(src, ice::data_type_from_c(dtype), out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_bitcast_to =
                [](void* plugin_context,
                   const TF_Tensor_Handle* src,
                   TF_DataType_Enum dtype,
                   TF_Tensor_Handle** out,
                   TF_Status* status)
            {
                auto res = Tensor::create(plugin_context)
                               ->bitcast_to(src, ice::data_type_from_c(dtype), out);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .tensor_copy =
                [](void* plugin_context, TF_Tensor_Handle* src, TF_Tensor_Handle* dst)
            {
                Tensor::create(plugin_context)->copy(src, dst);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
