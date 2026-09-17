#ifndef CONGELADO_C_GENERATOR_BLOCK_H_
#define CONGELADO_C_GENERATOR_BLOCK_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/definition.h"
#include "c/extern/generator/node.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFGeneratorBlock
    {
        void* plugin_data;
    } TFGeneratorBlock;

    typedef struct TFGeneratorBlockOps
    {
        size_t struct_size;
        void (*destroy)(TFGeneratorBlock* block);
        void (*get_name)(TFGeneratorBlock* block, TF_String* out_name);

        void (*add_node)(
            TFGeneratorBlock* block,
            TFGeneratorDefinition* definition,
            TFGeneratorNode* out_node,
            TF_Status* out_status
        );

        void (*get_node)(TFGeneratorBlock* block, int index, TFGeneratorNode* out_node, TF_Status* out_status);

        void (*list_nodes)(TFGeneratorBlock* block, TF_Tensor** out_nodes, TF_Status* out_status);

        void (*set_name)(TFGeneratorBlock* block, const TF_String* name);
    } TFGeneratorBlockOps;

#define TF_GENERATOR_BLOCK_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorBlockOps, set_name)

    TF_CAPI_EXPORT void
    create_generator_block(TFGeneratorBlockOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_block(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_BLOCK_H_
