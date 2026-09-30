#ifndef TENSORFLOW_C_EXTERN_OPS_OP_DEFINITION_BUILDER_H_
#define TENSORFLOW_C_EXTERN_OPS_OP_DEFINITION_BUILDER_H_

#include "include/c/macros.h"
#include "include/c/intern/tstring.h"
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
        void (*add_attr)(TF_OpDefinitionBuilder* builder, const TF_String* attr_spec);
        void (*add_input)(TF_OpDefinitionBuilder* builder, const TF_String* input_spec);
        void (*add_output)(TF_OpDefinitionBuilder* builder, const TF_String* output_spec);
        void (*set_is_commutative)(TF_OpDefinitionBuilder* builder, bool is_commutative);
        void (*set_is_aggregate)(TF_OpDefinitionBuilder* builder, bool is_aggregate);
        void (*set_is_stateful)(TF_OpDefinitionBuilder* builder, bool is_stateful);
        void (*set_allows_uninitialized_input)(TF_OpDefinitionBuilder* builder, bool allows_uninitialized_input);
        void (*deprecated)(TF_OpDefinitionBuilder* builder, int version, const TF_String* explanation);
        void (*set_shape_inference_function)(TF_OpDefinitionBuilder* builder, void (*shape_inference_func)(TF_ShapeInferenceContext* ctx, TF_Status* out_status));
        void (*register_op_definition)(TF_OpDefinitionBuilder* builder, TF_Status* out_status);
    } TF_OpDefinitionBuilderOps;
    #define TF_OP_DEFINITION_BUILDER_STRUCT_SIZE TF_OFFSET_OF_END(TF_OpDefinitionBuilderOps, register_op_definition)
    
    TF_CAPI_EXPORT void create_op_definition_builder(TF_OpDefinitionBuilderOps** ops, void** plugin_context, const TF_String* op_name, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_op_definition_builder(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_OP_DEFINITION_BUILDER_H_
