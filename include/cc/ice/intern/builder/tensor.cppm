// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"

export module cc_ice_intern_builder:tensor;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TF_TensorOps
{
public:
    explicit TF_TensorOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_TensorOps* TF_TensorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_TensorOps_ops = TF_TensorOps_ops;
    }

    TF_TensorOps(const TF_TensorOps&) = delete;
    TF_TensorOps& operator=(const TF_TensorOps&) = delete;

    static TF_TensorOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_TensorOps*>(ctx);
    }

    template<typename HandleT>
    static TF_TensorOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_TensorOps*>(handle->plugin_data);
    }

    virtual ~TF_TensorOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;
    virtual void set_dtype(TFDataTypeEnum dtype) noexcept = 0;
    virtual void set_dims(const int64_t* dims, int num_dims) noexcept = 0;
    virtual void set_byte_size(size_t len) noexcept = 0;
    virtual void delete_tensor() noexcept = 0;
    virtual void tensor_type(TFDataTypeEnum* out_dtype) noexcept = 0;
    virtual void num_dims(int* out_num_dims) noexcept = 0;
    virtual void dim(int dim_index, int64_t* out_dim) noexcept = 0;
    virtual void tensor_element_count(int64_t* out_count) noexcept = 0;
    virtual void tensor_byte_size(size_t* out_byte_size) noexcept = 0;
    virtual void tensor_data(void** out_data) noexcept = 0;
    virtual void tensor_bitcast_from(
        TFDataTypeEnum dtype,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void tensor_bitcast_to(
        TFDataTypeEnum dtype,
        TF_Tensor** out_tensor,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void tensor_copy(const ice::sonic::TF_TensorOps& dst) noexcept = 0;
    virtual void set_strides(const int64_t* strides, int num_strides) noexcept = 0;
    virtual void stride(int dim_index, int64_t* out_stride) noexcept = 0;
    virtual void set_storage_offset(int64_t offset_elements) noexcept = 0;
    virtual void storage_offset(int64_t* out_offset_elements) noexcept = 0;
    virtual void set_device_index(int device_index) noexcept = 0;
    virtual void get_device_index(int* out_device_index) noexcept = 0;
    virtual void tensor_view(
        const int64_t* dims,
        int num_dims,
        const int64_t* strides,
        int64_t offset_elements,
        TF_Tensor** out_view,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_Tensor*)) noexcept
    {
        m_vtable = ::TF_TensorOps{
            .struct_size = TF_OFFSET_OF_END(::TF_TensorOps, tensor_view),

            .create = create,
            .destroy =
                [](TF_Tensor* handle) noexcept
            {
                auto& self = TF_TensorOps::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](TF_Tensor* tensor, TF_String* out_name) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },
            .set_dtype =
                [](TF_Tensor* tensor, TFDataTypeEnum dtype) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.set_dtype(dtype);
            },
            .set_dims =
                [](TF_Tensor* tensor, const int64_t* dims, int num_dims) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.set_dims(dims, num_dims);
            },
            .set_byte_size =
                [](TF_Tensor* tensor, size_t len) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.set_byte_size(len);
            },
            .delete_tensor =
                [](TF_Tensor* tensor) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.delete_tensor();
            },
            .tensor_type =
                [](const TF_Tensor* tensor, TFDataTypeEnum* out_dtype) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.tensor_type(out_dtype);
            },
            .num_dims =
                [](const TF_Tensor* tensor, int* out_num_dims) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.num_dims(out_num_dims);
            },
            .dim =
                [](const TF_Tensor* tensor, int dim_index, int64_t* out_dim) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.dim(dim_index, out_dim);
            },
            .tensor_element_count =
                [](const TF_Tensor* tensor, int64_t* out_count) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.tensor_element_count(out_count);
            },
            .tensor_byte_size =
                [](const TF_Tensor* tensor, size_t* out_byte_size) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.tensor_byte_size(out_byte_size);
            },
            .tensor_data =
                [](const TF_Tensor* tensor, void** out_data) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.tensor_data(out_data);
            },
            .tensor_bitcast_from =
                [](TF_Tensor* src,
                   TFDataTypeEnum dtype,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_TensorOps::from_handle(src);
                self.tensor_bitcast_from(
                    dtype,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .tensor_bitcast_to =
                [](const TF_Tensor* src,
                   TFDataTypeEnum dtype,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_TensorOps::from_handle(src);
                self.tensor_bitcast_to(
                    dtype,
                    out_tensor,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .tensor_copy =
                [](TF_Tensor* src, TF_Tensor* dst) noexcept
            {
                auto& self = TF_TensorOps::from_handle(src);
                self.tensor_copy(self.wrap(std::type_identity<ice::sonic::TF_TensorOps>{}, dst));
            },
            .set_strides =
                [](TF_Tensor* tensor, const int64_t* strides, int num_strides) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.set_strides(strides, num_strides);
            },
            .stride =
                [](const TF_Tensor* tensor, int dim_index, int64_t* out_stride) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.stride(dim_index, out_stride);
            },
            .set_storage_offset =
                [](TF_Tensor* tensor, int64_t offset_elements) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.set_storage_offset(offset_elements);
            },
            .storage_offset =
                [](const TF_Tensor* tensor, int64_t* out_offset_elements) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.storage_offset(out_offset_elements);
            },
            .set_device_index =
                [](TF_Tensor* tensor, int device_index) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.set_device_index(device_index);
            },
            .get_device_index =
                [](const TF_Tensor* tensor, int* out_device_index) noexcept
            {
                auto& self = TF_TensorOps::from_handle(tensor);
                self.get_device_index(out_device_index);
            },
            .tensor_view =
                [](TF_Tensor* base,
                   const int64_t* dims,
                   int num_dims,
                   const int64_t* strides,
                   int64_t offset_elements,
                   TF_Tensor** out_view,
                   TF_Status* out_status) noexcept
            {
                auto& self = TF_TensorOps::from_handle(base);
                self.tensor_view(
                    dims,
                    num_dims,
                    strides,
                    offset_elements,
                    out_view,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::TF_TensorOps
    wrap(std::type_identity<ice::sonic::TF_TensorOps>, const ::TF_Tensor* handle) const noexcept
    {
        return ice::sonic::TF_TensorOps{m_TF_TensorOps_ops, const_cast<::TF_Tensor*>(handle)};
    }

    const ::TF_TensorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_Tensor& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TF_TensorOps*>(&m_vtable)
        );
    }

private:
    ::TF_TensorOps m_vtable;
    ::TF_Tensor m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_TensorOps* m_TF_TensorOps_ops{nullptr};
};

} // namespace ice::builder
