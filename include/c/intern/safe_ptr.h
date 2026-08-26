/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_SAFE_PTR_H_
#define TENSORFLOW_C_SAFE_PTR_H_

#include "c/abi/api.h"

#include <memory>

// C++ RAII wrappers for C ABI types.
// These connect C++ code to the C ABI by providing automatic resource cleanup.

namespace tensorflow {
namespace detail {

    struct TFTensorDeleter
    {
        void operator()(TF_Tensor* p) const;
    };

    struct TFStatusDeleter
    {
        void operator()(TF_Status* p) const;
    };

    struct TFBufferDeleter
    {
        void operator()(TF_Buffer* p) const;
    };

} // namespace detail

// Safe container for an owned TF_Tensor. Auto-deleted on destruction.
using Safe_TF_TensorPtr = std::unique_ptr<TF_Tensor, detail::TFTensorDeleter>;
Safe_TF_TensorPtr make_safe(TF_Tensor* tensor);

// Safe container for an owned TF_Status. Auto-deleted on destruction.
using Safe_TF_StatusPtr = std::unique_ptr<TF_Status, detail::TFStatusDeleter>;
Safe_TF_StatusPtr make_safe(TF_Status* status);

// Safe container for an owned TF_Buffer. Auto-deleted on destruction.
using Safe_TF_BufferPtr = std::unique_ptr<TF_Buffer, detail::TFBufferDeleter>;
Safe_TF_BufferPtr make_safe(TF_Buffer* buffer);

} // namespace tensorflow

#endif // TENSORFLOW_C_SAFE_PTR_H_
