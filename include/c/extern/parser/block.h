#ifndef CONGELADO_C_PARSER_BLOCK_H_
#define CONGELADO_C_PARSER_BLOCK_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/parser/node.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserBlock { void* plugin_data; } TFParserBlock;

    typedef struct TFParserBlockOps {
        size_t struct_size;
        void (*create)(TFParserBlock* out_handle);
        void (*destroy)(TFParserBlock* handle);
        void (*get_name)(TFParserBlock* block, TF_String* out_name, TF_Status* out_status);
        void (*get_node_count)(TFParserBlock* block, int* out_count, TF_Status* out_status);
        void (*get_node)(TFParserBlock* block, int index, TFParserNode* out_node, TF_Status* out_status);
    } TFParserBlockOps;
    #define TF_PARSER_BLOCK_STRUCT_SIZE TF_OFFSET_OF_END(TFParserBlockOps, get_node)

    TF_CAPI_EXPORT void create_parser_block(TFParserBlockOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_block(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
