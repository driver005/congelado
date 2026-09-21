import os

files = {
    'include/c/extern/ops/shape_handle.h': '''#ifndef TENSORFLOW_C_EXTERN_OPS_SHAPE_HANDLE_H_
#define TENSORFLOW_C_EXTERN_OPS_SHAPE_HANDLE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_ShapeHandle { void* plugin_data; } TF_ShapeHandle;

    // TF_ShapeHandleOps
    typedef struct TF_ShapeHandleOps {
        size_t struct_size;
    } TF_ShapeHandleOps;
    #define TF_SHAPE_HANDLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ShapeHandleOps, struct_size)
    TF_CAPI_EXPORT void create_shape_handle(TF_ShapeHandleOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_shape_handle(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_SHAPE_HANDLE_H_
''',

    'include/c/extern/ops/dimension_handle.h': '''#ifndef TENSORFLOW_C_EXTERN_OPS_DIMENSION_HANDLE_H_
#define TENSORFLOW_C_EXTERN_OPS_DIMENSION_HANDLE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_DimensionHandle { void* plugin_data; } TF_DimensionHandle;

    // TF_DimensionHandleOps
    typedef struct TF_DimensionHandleOps {
        size_t struct_size;
        void (*value_known)(TF_DimensionHandle* dim_handle, int* out_known);
        void (*value)(TF_DimensionHandle* dim_handle, int64_t* out_value);
    } TF_DimensionHandleOps;
    #define TF_DIMENSION_HANDLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DimensionHandleOps, value)
    TF_CAPI_EXPORT void create_dimension_handle(TF_DimensionHandleOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_dimension_handle(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_DIMENSION_HANDLE_H_
''',

    'include/c/extern/ops/shape_inference_context.h': '''#ifndef TENSORFLOW_C_EXTERN_OPS_SHAPE_INFERENCE_CONTEXT_H_
#define TENSORFLOW_C_EXTERN_OPS_SHAPE_INFERENCE_CONTEXT_H_

#include "include/c/macros.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"
#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/dimension_handle.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_ShapeInferenceContext { void* plugin_data; } TF_ShapeInferenceContext;

    // TF_ShapeInferenceContextOps
    typedef struct TF_ShapeInferenceContextOps {
        size_t struct_size;
        void (*num_inputs)(TF_ShapeInferenceContext* ctx, int64_t* out_num);
        void (*get_input)(TF_ShapeInferenceContext* ctx, int i, TF_ShapeHandle* handle, TF_Status* out_status);
        void (*set_output)(TF_ShapeInferenceContext* ctx, int i, TF_ShapeHandle* handle, TF_Status* out_status);
        void (*scalar)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle);
        void (*vector_from_size)(TF_ShapeInferenceContext* ctx, size_t size, TF_ShapeHandle* handle);
        void (*get_attr_type)(TF_ShapeInferenceContext* ctx, const char* attr_name, TFDataTypeEnum* out_val, TF_Status* out_status);
        void (*rank)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t* out_rank);
        void (*rank_known)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int* out_known);
        void (*with_rank)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result, TF_Status* out_status);
        void (*with_rank_at_least)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result, TF_Status* out_status);
        void (*with_rank_at_most)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle, int64_t rank, TF_ShapeHandle* result, TF_Status* out_status);
        void (*dim)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* shape_handle, int64_t i, TF_DimensionHandle* result);
        void (*subshape)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* shape_handle, int64_t start, int64_t end, TF_ShapeHandle* result, TF_Status* out_status);
        void (*set_unknown_shape)(TF_ShapeInferenceContext* ctx, TF_Status* out_status);
        void (*concatenate_shapes)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* first, TF_ShapeHandle* second, TF_ShapeHandle* result, TF_Status* out_status);
    } TF_ShapeInferenceContextOps;
    #define TF_SHAPE_INFERENCE_CONTEXT_STRUCT_SIZE TF_OFFSET_OF_END(TF_ShapeInferenceContextOps, concatenate_shapes)
    TF_CAPI_EXPORT void create_shape_inference_context(TF_ShapeInferenceContextOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_shape_inference_context(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_SHAPE_INFERENCE_CONTEXT_H_
''',

    'include/c/extern/ops/op_definition_builder.h': '''#ifndef TENSORFLOW_C_EXTERN_OPS_OP_DEFINITION_BUILDER_H_
#define TENSORFLOW_C_EXTERN_OPS_OP_DEFINITION_BUILDER_H_

#include "include/c/macros.h"
#include "include/c/intern/datatype.h"
#include "include/c/intern/status.h"
#include "include/c/extern/ops/shape_inference_context.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_OpDefinitionBuilder { void* plugin_data; } TF_OpDefinitionBuilder;

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
        void (*set_shape_inference_function)(TF_OpDefinitionBuilder* builder, void (*shape_inference_func)(TF_ShapeInferenceContext* ctx, TF_Status* out_status));
        void (*register_op_definition)(TF_OpDefinitionBuilder* builder, TF_Status* out_status);
    } TF_OpDefinitionBuilderOps;
    #define TF_OP_DEFINITION_BUILDER_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpDefinitionBuilderOps, register_op_definition)
    
    TF_CAPI_EXPORT void create_op_definition_builder(TF_OpDefinitionBuilderOps** ops, void** plugin_context, const char* op_name, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_op_definition_builder(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_OP_DEFINITION_BUILDER_H_
''',

    'include/c/extern/ops/ops.h': '''#ifndef TENSORFLOW_C_EXTERN_OPS_OPS_H_
#define TENSORFLOW_C_EXTERN_OPS_OPS_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/ops/shape_handle.h"
#include "include/c/extern/ops/dimension_handle.h"
#include "include/c/extern/ops/shape_inference_context.h"
#include "include/c/extern/ops/op_definition_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Ops {
        void* plugin_data;
        const TF_ShapeHandleOps* shape_handle_ops;
        const TF_DimensionHandleOps* dimension_handle_ops;
        const TF_ShapeInferenceContextOps* shape_inference_context_ops;
        const TF_OpDefinitionBuilderOps* op_definition_builder_ops;
    } TF_Ops;

    typedef struct TF_OpsOps {
        size_t struct_size;
        void (*destroy)(TF_Ops* ops_facade);
        void (*get_name)(TF_Ops* ops_facade, TF_String* out_name);
    } TF_OpsOps;

    #define TF_OPS_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpsOps, get_name)

    TF_CAPI_EXPORT void create_ops(TF_OpsOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_ops(void* plugin_context);

    static inline void init_ops(TF_OpsOps** ops, TF_Ops* ops_facade, TF_Status* out_status) {
        create_ops(ops, &ops_facade->plugin_data, out_status);

        TF_ShapeHandleOps* shape_handle_ops = NULL;
        create_shape_handle(&shape_handle_ops, &ops_facade->plugin_data, out_status);
        ops_facade->shape_handle_ops = shape_handle_ops;

        TF_DimensionHandleOps* dimension_handle_ops = NULL;
        create_dimension_handle(&dimension_handle_ops, &ops_facade->plugin_data, out_status);
        ops_facade->dimension_handle_ops = dimension_handle_ops;

        TF_ShapeInferenceContextOps* shape_inference_context_ops = NULL;
        create_shape_inference_context(&shape_inference_context_ops, &ops_facade->plugin_data, out_status);
        ops_facade->shape_inference_context_ops = shape_inference_context_ops;

        TF_OpDefinitionBuilderOps* op_definition_builder_ops = NULL;
        create_op_definition_builder(&op_definition_builder_ops, &ops_facade->plugin_data, "", out_status);
        ops_facade->op_definition_builder_ops = op_definition_builder_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_OPS_H_
'''
}

for filepath, content in files.items():
    with open(filepath, 'w') as f:
        f.write(content)
