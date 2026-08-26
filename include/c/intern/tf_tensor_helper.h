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

#ifndef TENSORFLOW_C_TF_TENSOR_HELPER_H_
#define TENSORFLOW_C_TF_TENSOR_HELPER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_datatype.h"
#include "c/intern/tf_tensor.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_TensorBitcastFrom changes the data type of a tensor without copying data.
    TF_CAPI_EXPORT extern void
    TF_TensorBitcastFrom(TF_Tensor* src, TF_DataType dtype, TF_Tensor** out);

    // TF_TensorBitcastTo changes the data type of a tensor without copying data.
    TF_CAPI_EXPORT extern void
    TF_TensorBitcastTo(const TF_Tensor* src, TF_DataType dtype, TF_Tensor** out);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TENSOR_HELPER_H_
