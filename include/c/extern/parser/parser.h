#ifndef CONGELADO_C_PARSER_H_
#define CONGELADO_C_PARSER_H_

#include "include/c/extern/parser/attribute.h"
#include "include/c/extern/parser/block.h"
#include "include/c/extern/parser/catalog.h"
#include "include/c/extern/parser/definition.h"
#include "include/c/extern/parser/function.h"
#include "include/c/extern/parser/module.h"
#include "include/c/extern/parser/node.h"
#include "include/c/extern/parser/parameter.h"
#include "include/c/extern/parser/typeinfo.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Parser
    {
        void* plugin_data;
        const TFParserCatalogOps* catalog_ops;
        const TFParserModuleOps* module_ops;
        const TFParserFunctionOps* function_ops;
        const TFParserParameterOps* parameter_ops;
        const TFParserTypeInfoOps* typeinfo_ops;
        const TFParserAttributeOps* attribute_ops;
        const TFParserDefinitionOps* definition_ops;
        const TFParserBlockOps* block_ops;
        const TFParserNodeOps* node_ops;
    } TF_Parser;

    typedef struct TF_ParserOps
    {
        size_t struct_size;
        void (*create)(TF_Parser* out_handle);
        void (*destroy)(TF_Parser* handle);
        void (*get_name)(TF_Parser* parser, TF_String* out_name);
    } TF_ParserOps;

#define TF_PARSER_STRUCT_SIZE TF_OFFSET_OF_END(TF_ParserOps, get_name)

    TF_CAPI_EXPORT void
    create_parser(TF_ParserOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser(void* plugin_context);

    static inline void init_parser(TF_ParserOps** ops, TF_Parser* parser, TF_Status* out_status)
    {
        create_parser(ops, &parser->plugin_data, out_status);

        TFParserCatalogOps* catalog_ops = NULL;
        create_parser_catalog(&catalog_ops, &parser->plugin_data, out_status);
        parser->catalog_ops = catalog_ops;

        TFParserModuleOps* module_ops = NULL;
        create_parser_module(&module_ops, &parser->plugin_data, out_status);
        parser->module_ops = module_ops;

        TFParserFunctionOps* function_ops = NULL;
        create_parser_function(&function_ops, &parser->plugin_data, out_status);
        parser->function_ops = function_ops;

        TFParserParameterOps* parameter_ops = NULL;
        create_parser_parameter(&parameter_ops, &parser->plugin_data, out_status);
        parser->parameter_ops = parameter_ops;

        TFParserTypeInfoOps* typeinfo_ops = NULL;
        create_parser_typeinfo(&typeinfo_ops, &parser->plugin_data, out_status);
        parser->typeinfo_ops = typeinfo_ops;

        TFParserAttributeOps* attribute_ops = NULL;
        create_parser_attribute(&attribute_ops, &parser->plugin_data, out_status);
        parser->attribute_ops = attribute_ops;

        TFParserDefinitionOps* definition_ops = NULL;
        create_parser_definition(&definition_ops, &parser->plugin_data, out_status);
        parser->definition_ops = definition_ops;

        TFParserBlockOps* block_ops = NULL;
        create_parser_block(&block_ops, &parser->plugin_data, out_status);
        parser->block_ops = block_ops;

        TFParserNodeOps* node_ops = NULL;
        create_parser_node(&node_ops, &parser->plugin_data, out_status);
        parser->node_ops = node_ops;
    }

#ifdef __cplusplus
}
#endif
#endif
