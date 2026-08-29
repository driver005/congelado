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
#include "c/intern/tf_status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Shape_Data — passive value type carrying a shape's dimensions.
    typedef struct TF_Shape_Data
    {
        size_t struct_size;
        int64_t* dims;
        int num_dims;
    } TF_Shape_Data;

#define TF_SHAPE_DATA_STRUCT_SIZE TF_OFFSET_OF_END(TF_Shape_Data, num_dims)

    // Legacy alias kept for call sites that refer to the data struct as TF_Shape.
    typedef TF_Shape_Data TF_Shape_Value;

    static inline void TF_ShapeDataInit(TF_Shape_Data* shape)
    {
        shape->struct_size = TF_SHAPE_DATA_STRUCT_SIZE;
        shape->dims        = NULL;
        shape->num_dims    = 0;
    }

    static inline void TF_ShapeDataDealloc(TF_Shape_Data* shape)
    {
        if (!shape) return;
        free(shape->dims);
        free(shape);
    }

    // Global helper functions that operate on TF_Shape_Data values.
    TF_CAPI_EXPORT extern TF_Shape_Data* TF_NewShapeData(const int64_t* dims, int num_dims);
    TF_CAPI_EXPORT extern void           TF_DeleteShapeData(TF_Shape_Data* shape);
    TF_CAPI_EXPORT extern int            TF_ShapeDataNumDims(const TF_Shape_Data* shape);
    TF_CAPI_EXPORT extern int64_t        TF_ShapeDataDim(const TF_Shape_Data* shape, int index);

    // --------------------------------------------------------------------------
    // TF_Shape — plugin vtable for shape operations.
    //
    // TF_Shape_Handle is an opaque pointer to a plugin-owned shape object.
    typedef struct TF_Shape_Handle TF_Shape_Handle;

    // Plugin-facing vtable registered via TF_InitShape.
    typedef struct TF_Shape
    {
        size_t struct_size;

        // Allocate a new shape (pass dims=NULL/num_dims=0 for scalar).
        // Must be freed with TF_DeleteShape.
        TF_Shape_Handle* (*TF_NewShape)(
            void* ctx, const int64_t* dims, int num_dims
        );

        // Free a handle returned by TF_NewShape.
        void (*TF_DeleteShape)(void* ctx, TF_Shape_Handle* shape);

        // Return the number of dimensions (-1 for unknown rank).
        int (*TF_ShapeNumDims)(void* ctx, const TF_Shape_Handle* shape);

        // Return the size of the given dimension (-1 for unknown).
        int64_t (*TF_ShapeDim)(void* ctx, const TF_Shape_Handle* shape, int index);

    } TF_Shape;

#define TF_SHAPE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Shape, TF_ShapeDim)

    TF_CAPI_EXPORT extern void TF_InitShape(
        TF_Shape** ops, void** plugin_context, TF_Status* status
    );

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SHAPE_H_
