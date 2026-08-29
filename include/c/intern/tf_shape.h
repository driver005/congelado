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
#include "c/intern/tf_tstring.h"

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

    static inline void shape_data_init(TF_Shape_Data* shape)
    {
        shape->struct_size = TF_SHAPE_DATA_STRUCT_SIZE;
        shape->dims = NULL;
        shape->num_dims = 0;
    }

    static inline void shape_data_dealloc(TF_Shape_Data* shape)
    {
        if (!shape) {
            return;
        }
        free(shape->dims);
        free(shape);
    }

    // Global helper functions that operate on TF_Shape_Data values.
    TF_CAPI_EXPORT TF_Shape_Data* new_shape_data(const int64_t* dims, int num_dims);
    TF_CAPI_EXPORT void delete_shape_data(TF_Shape_Data* shape);
    TF_CAPI_EXPORT int shape_data_num_dims(const TF_Shape_Data* shape);
    TF_CAPI_EXPORT int64_t shape_data_dim(const TF_Shape_Data* shape, int index);

    // --------------------------------------------------------------------------
    // TF_Shape — plugin vtable for shape operations.
    //
    // TF_Shape_Handle is an opaque pointer to a plugin-owned shape object.
    typedef struct TF_Shape_Handle TF_Shape_Handle;

    // Plugin-facing vtable registered via init_shape.
    typedef struct TF_Shape
    {
        size_t struct_size;

        // Return the backend's name (e.g. "shape") into *out.
        void (*get_name)(void* plugin_context, TF_String* out);

        // Allocate a new shape (pass dims=NULL/num_dims=0 for scalar).
        // Must be freed with TF_DeleteShape.
        TF_Shape_Handle* (*new_shape)(void* plugin_context, const int64_t* dims, int num_dims);

        // Free a handle returned by TF_NewShape.
        void (*delete_shape)(void* plugin_context, TF_Shape_Handle* shape);

        // Return the number of dimensions (-1 for unknown rank).
        int (*shape_num_dims)(void* plugin_context, const TF_Shape_Handle* shape);

        // Return the size of the given dimension (-1 for unknown).
        int64_t (*shape_dim)(void* plugin_context, const TF_Shape_Handle* shape, int index);

    } TF_Shape;

#define TF_SHAPE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Shape, shape_dim)

    TF_CAPI_EXPORT void init_shape(TF_Shape** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SHAPE_H_
