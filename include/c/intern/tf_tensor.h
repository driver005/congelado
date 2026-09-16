/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_C_TF_TENSOR_H_
#define TENSORFLOW_C_TF_TENSOR_H_

#include "c/macros.h"
#include "c/intern/tf_datatype.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Tensor — plugin vtable for tensor operations. TF_Tensor is an opaque pointer to a plugin-owned tensor object. It doubles as the "list/array carrier" type at C ABI boundaries (e.g. filesystem paths, generator definitions).
    typedef struct TF_TensorOps TF_TensorOps;

    typedef struct TF_Tensor
    {
        void* plugin_data;
        const TF_TensorOps* ops;
    } TF_Tensor;

    // Plugin-facing vtable registered via create_tensor.
    typedef struct TF_TensorOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "tensor") into *out.
        void (*get_name)(TF_Tensor* tensor, TF_String* out);

        void (*set_dtype)(TF_Tensor* tensor, TF_DataType_Enum dtype);
        void (*set_dims)(TF_Tensor* tensor, const int64_t* dims, int num_dims);
        void (*set_byte_size)(TF_Tensor* tensor, size_t len);

        void (*delete_tensor)(TF_Tensor* tensor);

        // Return the element data type of the tensor.
        TF_DataType_Enum (*tensor_type)(const TF_Tensor* tensor);

        // Return the number of dimensions.
        int (*num_dims)(const TF_Tensor* tensor);

        // Return the size of the d-th dimension.
        int64_t (*dim)(const TF_Tensor* tensor, int dim_index);

        // Return the total element count across all dimensions.
        int64_t (*tensor_element_count)(const TF_Tensor* tensor);

        // Return the total byte size of the data buffer.
        size_t (*tensor_byte_size)(const TF_Tensor* tensor);

        // Return a pointer to the raw data buffer.
        void* (*tensor_data)(const TF_Tensor* tensor);

        // Reinterpret src's buffer as dtype and write result into *out. *out must be freed with TF_DeleteTensor.
        void (*tensor_bitcast_from)(
            TF_Tensor* src,
            TF_DataType_Enum dtype,
            TF_Tensor** out,
            TF_Status* status
        );

        // Same as tensor_bitcast_from but src is const.
        void (*tensor_bitcast_to)(
            const TF_Tensor* src,
            TF_DataType_Enum dtype,
            TF_Tensor** out,
            TF_Status* status
        );

        // Deep-copy src into dst (dst must already be allocated with matching shape/type).
        void (*tensor_copy)(TF_Tensor* src, TF_Tensor* dst);

    } TF_TensorOps;

#define TF_TENSOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_TensorOps, tensor_copy)

    TF_CAPI_EXPORT void create_tensor(TF_TensorOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_tensor(void* plugin_context);

    // Real implementation, not declared-only — calls create_tensor and fills in tensor->ops.
    static inline void init_tensor(TF_Tensor* tensor, TF_Status* status)
    {
        TF_TensorOps* ops = NULL;
        create_tensor(&ops, &tensor->plugin_data, status);
        tensor->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TENSOR_H_
