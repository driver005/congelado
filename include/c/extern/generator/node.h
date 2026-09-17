#ifndef CONGELADO_C_GENERATOR_NODE_H_
#define CONGELADO_C_GENERATOR_NODE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/definition.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFGeneratorNode
    {
        void* plugin_data;
    } TFGeneratorNode;

    typedef struct TFGeneratorNodeOps
    {
        size_t struct_size;
        void (*destroy)(TFGeneratorNode* node_context);
        void (*get_name)(TFGeneratorNode* node_context, TF_String* out_name);

        void (*set_operand)(
            TFGeneratorNode* node_context,
            int index,
            const TF_String* var_name,
            TF_Status* out_status
        );
        void (*set_output_name)(
            TFGeneratorNode* node_context,
            int index,
            const TF_String* var_name,
            TF_Status* out_status
        );
        void (*set_attr)(
            TFGeneratorNode* node_context,
            const TF_String* name,
            const void* value,
            size_t value_size,
            TF_Status* out_status
        );

        void (*get_operand)(TFGeneratorNode* node_context, int index, TF_String* out_operand);
        void (*get_output_name)(TFGeneratorNode* node_context, int index, TF_String* out_output_name);
        void (*get_definition)(TFGeneratorNode* node_context, TFGeneratorDefinition* out_definition, TF_Status* out_status);
    } TFGeneratorNodeOps;

#define TF_GENERATOR_NODE_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorNodeOps, get_definition)

    TF_CAPI_EXPORT void
    create_generator_node(TFGeneratorNodeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_node(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_NODE_H_
