#ifndef CONGELADO_C_PARSER_NODE_H_
#define CONGELADO_C_PARSER_NODE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/parser/attribute.h"
#include "include/c/extern/parser/definition.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserNode { void* plugin_data; } TFParserNode;

    typedef struct TFParserNodeOps {
        size_t struct_size;
        void (*get_name)(TFParserNode* node, TF_String* out_name, TF_Status* out_status);
        void (*get_op_type)(TFParserNode* node, TF_String* out_op_type, TF_Status* out_status);
        void (*get_attribute_count)(TFParserNode* node, int* out_count, TF_Status* out_status);
        void (*get_attribute)(TFParserNode* node, int index, TFParserAttribute* out_attribute, TF_Status* out_status);
        void (*get_definition)(TFParserNode* node, TFParserDefinition* out_definition, TF_Status* out_status);
    } TFParserNodeOps;
    #define TF_PARSER_NODE_STRUCT_SIZE TF_OFFSET_OF_END(TFParserNodeOps, get_definition)

    TF_CAPI_EXPORT void create_parser_node(TFParserNodeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_node(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
