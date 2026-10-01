#ifndef TENSORFLOW_C_EXTERN_OPS_SHAPE_INFERENCE_CONTEXT_H_
#define TENSORFLOW_C_EXTERN_OPS_SHAPE_INFERENCE_CONTEXT_H_

#include "include/c/macros.h"
#include "include/c/intern/tstring.h"
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
        void (*create)(TF_ShapeInferenceContext* out_handle);
        void (*destroy)(TF_ShapeInferenceContext* handle);
        void (*num_inputs)(TF_ShapeInferenceContext* ctx, int64_t* out_num);
        void (*get_input)(TF_ShapeInferenceContext* ctx, int i, TF_ShapeHandle* handle, TF_Status* out_status);
        void (*set_output)(TF_ShapeInferenceContext* ctx, int i, TF_ShapeHandle* handle, TF_Status* out_status);
        void (*scalar)(TF_ShapeInferenceContext* ctx, TF_ShapeHandle* handle);
        void (*vector_from_size)(TF_ShapeInferenceContext* ctx, size_t size, TF_ShapeHandle* handle);
        void (*get_attr_type)(TF_ShapeInferenceContext* ctx, const TF_String* attr_name, TFDataTypeEnum* out_val, TF_Status* out_status);
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
