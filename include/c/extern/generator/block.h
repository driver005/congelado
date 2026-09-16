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

    typedef struct TF_Generator_Block
    {
        void* plugin_data;
    } TF_Generator_Block;

    typedef struct TF_Generator_BlockOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator_Block* block);
        void (*get_name)(TF_Generator_Block* block, TF_String* out);

        TF_Generator_Node* (*add_node)(
            TF_Generator_Block* block,
            TF_Generator_Definition* definition,
            TF_Status* status
        );

        TF_Generator_Node* (*get_node)(TF_Generator_Block* block, int index);

        TF_Tensor* (*list_nodes)(TF_Generator_Block* block, TF_Status* status);

        void (*set_name)(TF_Generator_Block* block, const TF_String* name);
    } TF_Generator_BlockOps;

#define TF_GENERATOR_BLOCK_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_BlockOps, set_name)

    TF_CAPI_EXPORT void
    create_generator_block(TF_Generator_BlockOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_generator_block(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_BLOCK_H_
