/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_C_OP_REQUIRES_H_
#define TENSORFLOW_C_C_OP_REQUIRES_H_

#include "c/intern/tf_status.h"

// Convenience macros for asserting and handling exceptional conditions, for
// C structs, including `TF_OpKernelContext`, `TF_Status`, etc. This is analogous
// to the macros in tensorflow/core/framework/op_requires.h. Provided
// for plugin OpKernel developer's convenience.
//
// NOTE: This header is included by both C and C++ translation units — no
// namespace, no C++ constructs at file scope.

#ifdef __GNUC__
#    define TF_PREDICT_TRUE(x)  __builtin_expect(!!(x), 1)
#    define TF_PREDICT_FALSE(x) __builtin_expect(!!(x), 0)
#else
#    define TF_PREDICT_TRUE(x)  (x)
#    define TF_PREDICT_FALSE(x) (x)
#endif

#define C_OPKERNELCONTEXT_REQUIRES_OK(CTX, C_STATUS, __VA_ARGS__)                                  \
    do {                                                                                           \
        TF_Status* _s = (__VA_ARGS__);                                                             \
        if (!TF_PREDICT_TRUE(TF_GetCode(_s) == TF_OK)) {                                           \
            TF_SetStatus(C_STATUS, TF_GetCode(_s), TF_Message(_s));                                \
            TF_OpKernelContext_Failure(CTX, C_STATUS);                                             \
            TF_DeleteStatus(_s);                                                                   \
            return;                                                                                \
        }                                                                                          \
        TF_DeleteStatus(_s);                                                                       \
    } while (0)

#define TF_CLEANUP_AND_RETURN_IF_ERROR(C_STATUS, BUFFER, __VA_ARGS__)                              \
    do {                                                                                           \
        TF_Status* _s = (__VA_ARGS__);                                                             \
        if (TF_PREDICT_FALSE(TF_GetCode(_s) != TF_OK)) {                                           \
            TF_DeleteStatus(C_STATUS);                                                             \
            TF_DeleteBuffer(BUFFER);                                                               \
            return _s;                                                                             \
        }                                                                                          \
        TF_DeleteStatus(_s);                                                                       \
    } while (0)

#endif // TENSORFLOW_C_C_OP_REQUIRES_H_
