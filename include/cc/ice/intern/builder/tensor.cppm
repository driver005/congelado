// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/tensor.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/tensor.h"

export module cc_ice_intern_builder:tensor;

import std;

export namespace ice::builder {

class TF_TensorOps
{
public:
    TF_TensorOps() noexcept :
        m_handle{.plugin_data = this}
    {
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
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    tensor_bitcast_from(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    tensor_bitcast_to(TFDataTypeEnum dtype, TF_Tensor** out_tensor) noexcept = 0;
    virtual void tensor_copy(const ice::sonic::TF_TensorOps& dst) noexcept = 0;
    virtual void set_strides(const int64_t* strides, int num_strides) noexcept = 0;
    virtual void stride(int dim_index, int64_t* out_stride) noexcept = 0;
    virtual void set_storage_offset(int64_t offset_elements) noexcept = 0;
    virtual void storage_offset(int64_t* out_offset_elements) noexcept = 0;
    virtual void set_device_index(int device_index) noexcept = 0;
    virtual void get_device_index(int* out_device_index) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> tensor_view(
        const int64_t* dims,
        int num_dims,
        const int64_t* strides,
        int64_t offset_elements,
        TF_Tensor** out_view
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_TensorOps{
            .struct_size = TF_TENSOR_STRUCT_SIZE,
            .get_name =
                [](TF_Tensor* tensor, TF_String* out_name) noexcept
            {
                TF_TensorOps::from_handle(tensor).get_name(ice::sonic::String::wrap(out_name));
            },
            .set_dtype =
                [](TF_Tensor* tensor, TFDataTypeEnum dtype) noexcept
            {
                TF_TensorOps::from_handle(tensor).set_dtype(dtype);
            },
            .set_dims =
                [](TF_Tensor* tensor, const int64_t* dims, int num_dims) noexcept
            {
                TF_TensorOps::from_handle(tensor).set_dims(dims, num_dims);
            },
            .set_byte_size =
                [](TF_Tensor* tensor, size_t len) noexcept
            {
                TF_TensorOps::from_handle(tensor).set_byte_size(len);
            },
            .delete_tensor =
                [](TF_Tensor* tensor) noexcept
            {
                TF_TensorOps::from_handle(tensor).delete_tensor();
            },
            .tensor_type =
                [](const TF_Tensor* tensor, TFDataTypeEnum* out_dtype) noexcept
            {
                TF_TensorOps::from_handle(tensor).tensor_type(out_dtype);
            },
            .num_dims =
                [](const TF_Tensor* tensor, int* out_num_dims) noexcept
            {
                TF_TensorOps::from_handle(tensor).num_dims(out_num_dims);
            },
            .dim =
                [](const TF_Tensor* tensor, int dim_index, int64_t* out_dim) noexcept
            {
                TF_TensorOps::from_handle(tensor).dim(dim_index, out_dim);
            },
            .tensor_element_count =
                [](const TF_Tensor* tensor, int64_t* out_count) noexcept
            {
                TF_TensorOps::from_handle(tensor).tensor_element_count(out_count);
            },
            .tensor_byte_size =
                [](const TF_Tensor* tensor, size_t* out_byte_size) noexcept
            {
                TF_TensorOps::from_handle(tensor).tensor_byte_size(out_byte_size);
            },
            .tensor_data =
                [](const TF_Tensor* tensor, void** out_data) noexcept
            {
                TF_TensorOps::from_handle(tensor).tensor_data(out_data);
            },
            .tensor_bitcast_from =
                [](TF_Tensor* src,
                   TFDataTypeEnum dtype,
                   TF_Tensor** out_tensor,
                   TF_Status* out_status) noexcept
            {
                auto res = TF_TensorOps::from_handle(src).tensor_bitcast_from(dtype, out_tensor);
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
                auto res = TF_TensorOps::from_handle(src).tensor_bitcast_to(dtype, out_tensor);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .tensor_copy =
                [](TF_Tensor* src, TF_Tensor* dst) noexcept
            {
                TF_TensorOps::from_handle(src).tensor_copy(ice::sonic::TF_TensorOps::wrap(dst));
            },
            .set_strides =
                [](TF_Tensor* tensor, const int64_t* strides, int num_strides) noexcept
            {
                TF_TensorOps::from_handle(tensor).set_strides(strides, num_strides);
            },
            .stride =
                [](const TF_Tensor* tensor, int dim_index, int64_t* out_stride) noexcept
            {
                TF_TensorOps::from_handle(tensor).stride(dim_index, out_stride);
            },
            .set_storage_offset =
                [](TF_Tensor* tensor, int64_t offset_elements) noexcept
            {
                TF_TensorOps::from_handle(tensor).set_storage_offset(offset_elements);
            },
            .storage_offset =
                [](const TF_Tensor* tensor, int64_t* out_offset_elements) noexcept
            {
                TF_TensorOps::from_handle(tensor).storage_offset(out_offset_elements);
            },
            .set_device_index =
                [](TF_Tensor* tensor, int device_index) noexcept
            {
                TF_TensorOps::from_handle(tensor).set_device_index(device_index);
            },
            .get_device_index =
                [](const TF_Tensor* tensor, int* out_device_index) noexcept
            {
                TF_TensorOps::from_handle(tensor).get_device_index(out_device_index);
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
                auto res = TF_TensorOps::from_handle(base)
                               .tensor_view(dims, num_dims, strides, offset_elements, out_view);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TF_TensorOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Tensor& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_TensorOps m_vtable;
    TF_Tensor m_handle;
};

} // namespace ice::builder
