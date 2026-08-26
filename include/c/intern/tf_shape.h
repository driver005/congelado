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

#ifndef TENSORFLOW_C_TF_SHAPE_H_
#define TENSORFLOW_C_TF_SHAPE_H_

#include "c/abi/macros.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Shape is a C API for TensorFlow shapes.
    typedef struct TF_Shape
    {
        size_t struct_size;
        int64_t* dims;
        int num_dims;
    } TF_Shape;

#define TF_SHAPE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Shape, num_dims)

    static inline void TF_ShapeInit(TF_Shape* shape)
    {
        shape->struct_size = TF_SHAPE_STRUCT_SIZE;
        shape->dims = nullptr;
        shape->num_dims = 0;
    }

    static inline void TF_ShapeDealloc(TF_Shape* shape)
    {
        if (!shape) {
            return;
        }
        free(shape->dims);
        free(shape);
    }

    TF_CAPI_EXPORT extern TF_Shape* TF_NewShape(const int64_t* dims, int num_dims);
    TF_CAPI_EXPORT extern void TF_DeleteShape(TF_Shape* shape);
    TF_CAPI_EXPORT extern int TF_ShapeNumDims(const TF_Shape* shape);
    TF_CAPI_EXPORT extern int64_t TF_ShapeDim(const TF_Shape* shape, int index);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SHAPE_H_
