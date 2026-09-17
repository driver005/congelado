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

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TFShapeData — passive value type carrying a shape's dimensions.
    typedef struct TFShapeData
    {
        size_t struct_size;
        int64_t* dims;
        int num_dims;
    } TFShapeData;

#define TF_SHAPE_DATA_STRUCT_SIZE TF_OFFSET_OF_END(TFShapeData, num_dims)

    // Legacy alias kept for call sites that refer to the data struct as TF_Shape.
    typedef TFShapeData TFShapeValue;

    static inline void shape_data_init(TFShapeData* shape)
    {
        shape->struct_size = TF_SHAPE_DATA_STRUCT_SIZE;
        shape->dims = NULL;
        shape->num_dims = 0;
    }

    static inline void shape_data_dealloc(TFShapeData* shape)
    {
        if (!shape) {
            return;
        }
        free(shape->dims);
        free(shape);
    }

    // Global helper functions that operate on TFShapeData values.
    TF_CAPI_EXPORT TFShapeData* new_shape_data(const int64_t* dims, int num_dims);
    TF_CAPI_EXPORT void delete_shape_data(TFShapeData* shape);
    TF_CAPI_EXPORT int shape_data_num_dims(const TFShapeData* shape);
    TF_CAPI_EXPORT int64_t shape_data_dim(const TFShapeData* shape, int index);

    // TF_Shape — plugin vtable for shape operations.

    typedef struct TF_Shape
    {
        void* plugin_data;
    } TF_Shape;

    // Plugin-facing vtable registered via create_shape.
    typedef struct TF_ShapeOps
    {
        size_t struct_size;

        // Return the backend's name (e.g. "shape") into *out.
        void (*get_name)(TF_Shape* shape, TF_String* out_name);

        // Set the dims (pass dims=NULL/num_dims=0 for scalar).
        void (*set_dims)(TF_Shape* shape, const int64_t* dims, int num_dims);

        void (*delete_shape)(TF_Shape* shape);

        // Return the number of dimensions (-1 for unknown rank).
        void (*shape_num_dims)(const TF_Shape* shape, int* out_num_dims);

        // Return the size of the given dimension (-1 for unknown).
        void (*shape_dim)(const TF_Shape* shape, int index, int64_t* out_dim);

    } TF_ShapeOps;

#define TF_SHAPE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ShapeOps, shape_dim)

    TF_CAPI_EXPORT void create_shape(TF_ShapeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_shape(void* plugin_context);

    // Real implementation, not declared-only — calls create_shape
    static inline void init_shape(TF_Shape* shape, TF_Status* out_status)
    {
        TF_ShapeOps* ops = NULL;
        create_shape(&ops, &shape->plugin_data, out_status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SHAPE_H_
