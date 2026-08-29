module;

#include "c/intern/tf_tensor.h"

export module cc_abi_builder_intern:tensor;

import std;
import :datatype;

export namespace ice::builder {

class Tensor
{
public:
    virtual ~Tensor() = default;

    virtual TF_Tensor_Handle* allocate_tensor(DataTypeEnum dtype, const int64_t* dims, int num_dims, size_t len) = 0;
    virtual void              delete_tensor(TF_Tensor_Handle* tensor) = 0;
    virtual DataTypeEnum      get_type(const TF_Tensor_Handle* tensor) const = 0;
    virtual int               get_num_dims(const TF_Tensor_Handle* tensor) const = 0;
    virtual int64_t           get_dim(const TF_Tensor_Handle* tensor, int dim_index) const = 0;
    virtual int64_t           get_num_elements(const TF_Tensor_Handle* tensor) const = 0;
    virtual size_t            get_byte_size(const TF_Tensor_Handle* tensor) const = 0;
    virtual void*             get_data(const TF_Tensor_Handle* tensor) const = 0;

    // Typed accessor — avoids static_cast<T*> at call sites that know the element type.
    template<typename T>
    T* get_data_as(const TF_Tensor_Handle* tensor) const {
        return static_cast<T*>(get_data(tensor));
    }
    virtual void              bitcast_from(TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out) = 0;
    virtual void              bitcast_to(const TF_Tensor_Handle* src, DataTypeEnum dtype, TF_Tensor_Handle** out) = 0;
    virtual void              copy(TF_Tensor_Handle* src, TF_Tensor_Handle* dst) = 0;

    static TF_Tensor* get_generic_vtable() {
        static TF_Tensor vtable = {
            .struct_size = sizeof(TF_Tensor),
            .TF_AllocateTensor = [](void* ctx, TF_DataType_Enum dtype, const int64_t* dims, int num_dims, size_t len) -> TF_Tensor_Handle* {
                return ctx_as<Tensor>(ctx)->allocate_tensor(data_type_from_c(dtype), dims, num_dims, len);
            },
            .TF_DeleteTensor = [](void* ctx, TF_Tensor_Handle* tensor) {
                ctx_as<Tensor>(ctx)->delete_tensor(tensor);
            },
            .TF_TensorType = [](void* ctx, const TF_Tensor_Handle* tensor) -> TF_DataType_Enum {
                return data_type_to_c(ctx_as<Tensor>(ctx)->get_type(tensor));
            },
            .TF_NumDims = [](void* ctx, const TF_Tensor_Handle* tensor) -> int {
                return ctx_as<Tensor>(ctx)->get_num_dims(tensor);
            },
            .TF_Dim = [](void* ctx, const TF_Tensor_Handle* tensor, int dim_index) -> int64_t {
                return ctx_as<Tensor>(ctx)->get_dim(tensor, dim_index);
            },
            .TF_TensorElementCount = [](void* ctx, const TF_Tensor_Handle* tensor) -> int64_t {
                return ctx_as<Tensor>(ctx)->get_num_elements(tensor);
            },
            .TF_TensorByteSize = [](void* ctx, const TF_Tensor_Handle* tensor) -> size_t {
                return ctx_as<Tensor>(ctx)->get_byte_size(tensor);
            },
            .TF_TensorData = [](void* ctx, const TF_Tensor_Handle* tensor) -> void* {
                return ctx_as<Tensor>(ctx)->get_data(tensor);
            },
            .TF_TensorBitcastFrom = [](void* ctx, TF_Tensor_Handle* src, TF_DataType_Enum dtype, TF_Tensor_Handle** out) {
                ctx_as<Tensor>(ctx)->bitcast_from(src, data_type_from_c(dtype), out);
            },
            .TF_TensorBitcastTo = [](void* ctx, const TF_Tensor_Handle* src, TF_DataType_Enum dtype, TF_Tensor_Handle** out) {
                ctx_as<Tensor>(ctx)->bitcast_to(src, data_type_from_c(dtype), out);
            },
            .TF_TensorCopy = [](void* ctx, TF_Tensor_Handle* src, TF_Tensor_Handle* dst) {
                ctx_as<Tensor>(ctx)->copy(src, dst);
            }
        };
        return &vtable;
    }
};

} // namespace ice::builder
