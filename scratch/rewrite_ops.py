import re

ops_h_template = """/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_OPS_H_
#define TENSORFLOW_C_OPS_H_

#include "include/c/macros.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_ShapeHandle { void* plugin_data; } TF_ShapeHandle;
    typedef struct TF_DimensionHandle { void* plugin_data; } TF_DimensionHandle;
    typedef struct TF_ShapeInferenceContext { void* plugin_data; } TF_ShapeInferenceContext;
    typedef struct TF_OpDefinitionBuilder { void* plugin_data; } TF_OpDefinitionBuilder;

    // TF_ShapeHandleOps
    typedef struct TF_ShapeHandleOps {
        size_t struct_size;
    } TF_ShapeHandleOps;
    #define TF_SHAPE_HANDLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ShapeHandleOps, struct_size)
    TF_CAPI_EXPORT void create_shape_handle(TF_ShapeHandleOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_shape_handle(void* plugin_context);
    static inline void init_shape_handle(TF_ShapeHandle* handle, TF_Status* status) {
        TF_ShapeHandleOps* ops = NULL;
        create_shape_handle(&ops, &handle->plugin_data, status);
    }

    // TF_DimensionHandleOps
    typedef struct TF_DimensionHandleOps {
        size_t struct_size;
        int (*value_known)(TF_DimensionHandle* dim_handle);
        int64_t (*value)(TF_DimensionHandle* dim_handle);
    } TF_DimensionHandleOps;
    #define TF_DIMENSION_HANDLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DimensionHandleOps, value)
    TF_CAPI_EXPORT void create_dimension_handle(TF_DimensionHandleOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_dimension_handle(void* plugin_context);
    static inline void init_dimension_handle(TF_DimensionHandle* handle, TF_Status* status) {
        TF_DimensionHandleOps* ops = NULL;
        create_dimension_handle(&ops, &handle->plugin_data, status);
    }

    // TF_ShapeInferenceContextOps
    typedef struct TF_ShapeInferenceContextOps {
        size_t struct_size;
        int64_t (*num_inputs)(TF_ShapeInferenceContext* ctx);
        void (*get_input)(TF_ShapeInferenceContext* ctx, int i, TF_ShapeHandle* handle, TF_Status* status);
        void (*set_output)(TF_ShapeInferenceContext* ctx, int i, TF_ShapeHandle* handle, TF_Status* status);
        void (*scalar)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle);
        void (*vector_from_size)(TF_ShapeInferenceContext* ctx, size_t size, TF_ShapeHandle* handle);
        void (*get_attr_type)(TF_ShapeInferenceContext* ctx, const char* attr_name, TFDataTypeEnum* val, TF_Status* status);
        int64_t (*rank)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle);
        int (*rank_known)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle);
        void (*with_rank)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result, TF_Status* status);
        void (*with_rank_at_least)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result, TF_Status* status);
        void (*with_rank_at_most)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result, TF_Status* status);
        void (*dim)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* shape_handle, int64_t i, TF_DimensionHandle* result);
        void (*subshape)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* shape_handle, int64_t start, int64_t end, TF_ShapeHandle* result, TF_Status* status);
        void (*set_unknown_shape)(TF_ShapeInferenceContext* ctx, TF_Status* status);
        void (*concatenate_shapes)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* first, TF_ShapeHandle* second, TF_ShapeHandle* result, TF_Status* status);
    } TF_ShapeInferenceContextOps;
    #define TF_SHAPE_INFERENCE_CONTEXT_STRUCT_SIZE TF_OFFSET_OF_END(TF_ShapeInferenceContextOps, concatenate_shapes)
    TF_CAPI_EXPORT void create_shape_inference_context(TF_ShapeInferenceContextOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_shape_inference_context(void* plugin_context);
    static inline void init_shape_inference_context(TF_ShapeInferenceContext* handle, TF_Status* status) {
        TF_ShapeInferenceContextOps* ops = NULL;
        create_shape_inference_context(&ops, &handle->plugin_data, status);
    }

    // TF_OpDefinitionBuilderOps
    typedef struct TF_OpDefinitionBuilderOps {
        size_t struct_size;
        void (*add_attr)(TF_OpDefinitionBuilder* builder, const char* attr_spec);
        void (*add_input)(TF_OpDefinitionBuilder* builder, const char* input_spec);
        void (*add_output)(TF_OpDefinitionBuilder* builder, const char* output_spec);
        void (*set_is_commutative)(TF_OpDefinitionBuilder* builder, bool is_commutative);
        void (*set_is_aggregate)(TF_OpDefinitionBuilder* builder, bool is_aggregate);
        void (*set_is_stateful)(TF_OpDefinitionBuilder* builder, bool is_stateful);
        void (*set_allows_uninitialized_input)(TF_OpDefinitionBuilder* builder, bool allows_uninitialized_input);
        void (*deprecated)(TF_OpDefinitionBuilder* builder, int version, const char* explanation);
        void (*set_shape_inference_function)(TF_OpDefinitionBuilder* builder, void (*shape_inference_func)(TF_ShapeInferenceContext* ctx, TF_Status* status));
        void (*register_op_definition)(TF_OpDefinitionBuilder* builder, TF_Status* status);
    } TF_OpDefinitionBuilderOps;
    #define TF_OP_DEFINITION_BUILDER_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpDefinitionBuilderOps, register_op_definition)
    TF_CAPI_EXPORT void create_op_definition_builder(TF_OpDefinitionBuilderOps** ops, void** plugin_context, const char* op_name, TF_Status* status);
    TF_CAPI_EXPORT void destroy_op_definition_builder(void* plugin_context);
    static inline void init_op_definition_builder(TF_OpDefinitionBuilder* handle, const char* op_name, TF_Status* status) {
        TF_OpDefinitionBuilderOps* ops = NULL;
        create_op_definition_builder(&ops, &handle->plugin_data, op_name, status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_OPS_H_
"""

with open("include/c/ops.h", "w") as f:
    f.write(ops_h_template)

