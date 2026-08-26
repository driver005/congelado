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

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Tensor TF_Tensor;

    TF_CAPI_EXPORT extern TF_Tensor*
    TF_AllocateTensor(TF_DataType dtype, const int64_t* dims, int num_dims, size_t len);

    TF_CAPI_EXPORT extern void TF_DeleteTensor(TF_Tensor*);

    TF_CAPI_EXPORT extern TF_DataType TF_TensorType(const TF_Tensor*);

    TF_CAPI_EXPORT extern int TF_NumDims(const TF_Tensor*);

    TF_CAPI_EXPORT extern int64_t TF_Dim(const TF_Tensor*, int dim_index);

    TF_CAPI_EXPORT extern int64_t TF_TensorElementCount(const TF_Tensor*);

    TF_CAPI_EXPORT extern size_t TF_TensorByteSize(const TF_Tensor*);

    TF_CAPI_EXPORT extern void* TF_TensorData(const TF_Tensor*);

    TF_CAPI_EXPORT extern void
    TF_TensorBitcastFrom(TF_Tensor* src, TF_DataType dtype, TF_Tensor** out);

    TF_CAPI_EXPORT extern void
    TF_TensorBitcastTo(const TF_Tensor* src, TF_DataType dtype, TF_Tensor** out);

    TF_CAPI_EXPORT extern void TF_TensorCopy(TF_Tensor* src, TF_Tensor* dst);

    TF_CAPI_EXPORT extern void* TF_TensorData(const TF_Tensor*);

    TF_CAPI_EXPORT extern void TF_DeleteTensor(TF_Tensor*);

    TF_CAPI_EXPORT extern TF_DataType TF_TensorType(const TF_Tensor*);

    TF_CAPI_EXPORT extern int TF_NumDims(const TF_Tensor*);

    TF_CAPI_EXPORT extern int64_t TF_Dim(const TF_Tensor*, int dim_index);

    TF_CAPI_EXPORT extern int64_t TF_TensorElementCount(const TF_Tensor*);

    TF_CAPI_EXPORT extern size_t TF_TensorByteSize(const TF_Tensor*);

    TF_CAPI_EXPORT extern void* TF_TensorData(const TF_Tensor*);

    TF_CAPI_EXPORT extern void
    TF_TensorBitcastFrom(TF_Tensor* src, TF_DataType dtype, TF_Tensor** out);

    TF_CAPI_EXPORT extern void
    TF_TensorBitcastTo(const TF_Tensor* src, TF_DataType dtype, TF_Tensor** out);

    TF_CAPI_EXPORT extern void TF_TensorCopy(TF_Tensor* src, TF_Tensor* dst);

    TF_CAPI_EXPORT extern void* TF_TensorData(const TF_Tensor*);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TENSOR_H_
