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

#include "c/abi/macros.h"
#include "c/intern/tf_datatype.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_TensorOps — plugin vtable for tensor operations.
    //
    // TF_Tensor_Handle is an opaque pointer to a plugin-owned tensor object.
    // It doubles as the "list/array carrier" type at C ABI boundaries
    // (e.g. filesystem paths, generator definitions).
    typedef struct TF_Tensor_Handle TF_Tensor_Handle;

    // For backwards compatibility, TF_Tensor is also a typedef for the handle.
    typedef struct TF_Tensor_Handle TF_Tensor;

    // Plugin-facing vtable registered via init_tensor.
    typedef struct TF_TensorOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "tensor") into *out.
        void (*get_name)(void* plugin_context, TF_String* out);

        // Allocate a new tensor with the given dtype, shape and byte size.
        // The returned handle must be freed with TF_DeleteTensor.
        TF_Tensor_Handle* (*allocate_tensor)(
            void* plugin_context,
            TF_DataType_Enum dtype,
            const int64_t* dims,
            int num_dims,
            size_t len
        );

        // Free a handle returned by TF_AllocateTensor.
        void (*delete_tensor)(void* plugin_context, TF_Tensor_Handle* tensor);

        // Return the element data type of the tensor.
        TF_DataType_Enum (*tensor_type)(void* plugin_context, const TF_Tensor_Handle* tensor);

        // Return the number of dimensions.
        int (*num_dims)(void* plugin_context, const TF_Tensor_Handle* tensor);

        // Return the size of the d-th dimension.
        int64_t (*dim)(void* plugin_context, const TF_Tensor_Handle* tensor, int dim_index);

        // Return the total element count across all dimensions.
        int64_t (*tensor_element_count)(void* plugin_context, const TF_Tensor_Handle* tensor);

        // Return the total byte size of the data buffer.
        size_t (*tensor_byte_size)(void* plugin_context, const TF_Tensor_Handle* tensor);

        // Return a pointer to the raw data buffer.
        void* (*tensor_data)(void* plugin_context, const TF_Tensor_Handle* tensor);

        // Reinterpret src's buffer as dtype and write result into *out.
        // *out must be freed with TF_DeleteTensor.
        void (*tensor_bitcast_from)(
            void* plugin_context,
            TF_Tensor_Handle* src,
            TF_DataType_Enum dtype,
            TF_Tensor_Handle** out,
            TF_Status* status
        );

        // Same as tensor_bitcast_from but src is const.
        void (*tensor_bitcast_to)(
            void* plugin_context,
            const TF_Tensor_Handle* src,
            TF_DataType_Enum dtype,
            TF_Tensor_Handle** out,
            TF_Status* status
        );

        // Deep-copy src into dst (dst must already be allocated with matching shape/type).
        void (*tensor_copy)(void* plugin_context, TF_Tensor_Handle* src, TF_Tensor_Handle* dst);

    } TF_TensorOps;

#define TF_TENSOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_TensorOps, tensor_copy)

    TF_CAPI_EXPORT void init_tensor(TF_Tensor** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TENSOR_H_
