#ifndef TENSORFLOW_C_EXTERN_OPS_OPS_H_
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
        void (*create)(TF_Ops* out_handle);
        void (*destroy)(TF_Ops* handle);
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
        create_op_definition_builder(&op_definition_builder_ops, &ops_facade->plugin_data, NULL, out_status);
        ops_facade->op_definition_builder_ops = op_definition_builder_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_OPS_H_
