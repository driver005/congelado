import os

parser_dir = 'include/c/extern/parser'

# Delete old document.h if it exists
doc_h = os.path.join(parser_dir, 'document.h')
if os.path.exists(doc_h):
    os.remove(doc_h)

files = {}

# 1. catalog.h
files['catalog.h'] = """#ifndef CONGELADO_C_PARSER_CATALOG_H_
#define CONGELADO_C_PARSER_CATALOG_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/buffer.h"
#include "include/c/extern/parser/module.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserCatalog { void* plugin_data; } TFParserCatalog;

    typedef struct TFParserCatalogOps {
        size_t struct_size;
        void (*parse_file)(TFParserCatalog* catalog, const TF_String* file_path, TFParserModule* out_module, TF_Status* out_status);
        void (*parse_buffer)(TFParserCatalog* catalog, const TF_Buffer* buffer, TFParserModule* out_module, TF_Status* out_status);
    } TFParserCatalogOps;
    #define TF_PARSER_CATALOG_STRUCT_SIZE TF_OFFSET_OF_END(TFParserCatalogOps, parse_buffer)

    TF_CAPI_EXPORT void create_parser_catalog(TFParserCatalogOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_catalog(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 2. module.h
files['module.h'] = """#ifndef CONGELADO_C_PARSER_MODULE_H_
#define CONGELADO_C_PARSER_MODULE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/parser/function.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserModule { void* plugin_data; } TFParserModule;

    typedef struct TFParserModuleOps {
        size_t struct_size;
        void (*get_name)(TFParserModule* module, TF_String* out_name, TF_Status* out_status);
        void (*get_function_count)(TFParserModule* module, int* out_count, TF_Status* out_status);
        void (*get_function)(TFParserModule* module, int index, TFParserFunction* out_function, TF_Status* out_status);
    } TFParserModuleOps;
    #define TF_PARSER_MODULE_STRUCT_SIZE TF_OFFSET_OF_END(TFParserModuleOps, get_function)

    TF_CAPI_EXPORT void create_parser_module(TFParserModuleOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_module(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 3. function.h
files['function.h'] = """#ifndef CONGELADO_C_PARSER_FUNCTION_H_
#define CONGELADO_C_PARSER_FUNCTION_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/parser/parameter.h"
#include "include/c/extern/parser/block.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserFunction { void* plugin_data; } TFParserFunction;

    typedef struct TFParserFunctionOps {
        size_t struct_size;
        void (*get_name)(TFParserFunction* function, TF_String* out_name, TF_Status* out_status);
        void (*get_parameter_count)(TFParserFunction* function, int* out_count, TF_Status* out_status);
        void (*get_parameter)(TFParserFunction* function, int index, TFParserParameter* out_parameter, TF_Status* out_status);
        void (*get_block_count)(TFParserFunction* function, int* out_count, TF_Status* out_status);
        void (*get_block)(TFParserFunction* function, int index, TFParserBlock* out_block, TF_Status* out_status);
    } TFParserFunctionOps;
    #define TF_PARSER_FUNCTION_STRUCT_SIZE TF_OFFSET_OF_END(TFParserFunctionOps, get_block)

    TF_CAPI_EXPORT void create_parser_function(TFParserFunctionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_function(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 4. parameter.h
files['parameter.h'] = """#ifndef CONGELADO_C_PARSER_PARAMETER_H_
#define CONGELADO_C_PARSER_PARAMETER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/parser/typeinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserParameter { void* plugin_data; } TFParserParameter;

    typedef struct TFParserParameterOps {
        size_t struct_size;
        void (*get_name)(TFParserParameter* parameter, TF_String* out_name, TF_Status* out_status);
        void (*get_typeinfo)(TFParserParameter* parameter, TFParserTypeInfo* out_typeinfo, TF_Status* out_status);
    } TFParserParameterOps;
    #define TF_PARSER_PARAMETER_STRUCT_SIZE TF_OFFSET_OF_END(TFParserParameterOps, get_typeinfo)

    TF_CAPI_EXPORT void create_parser_parameter(TFParserParameterOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_parameter(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 5. typeinfo.h
files['typeinfo.h'] = """#ifndef CONGELADO_C_PARSER_TYPEINFO_H_
#define CONGELADO_C_PARSER_TYPEINFO_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/tensor.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserTypeInfo { void* plugin_data; } TFParserTypeInfo;

    typedef struct TFParserTypeInfoOps {
        size_t struct_size;
        void (*get_dtype)(TFParserTypeInfo* typeinfo, int* out_dtype, TF_Status* out_status);
        void (*get_shape)(TFParserTypeInfo* typeinfo, int64_t** out_dims, int* out_num_dims, TF_Status* out_status);
    } TFParserTypeInfoOps;
    #define TF_PARSER_TYPEINFO_STRUCT_SIZE TF_OFFSET_OF_END(TFParserTypeInfoOps, get_shape)

    TF_CAPI_EXPORT void create_parser_typeinfo(TFParserTypeInfoOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_typeinfo(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 6. block.h
files['block.h'] = """#ifndef CONGELADO_C_PARSER_BLOCK_H_
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
"""

# 7. node.h
files['node.h'] = """#ifndef CONGELADO_C_PARSER_NODE_H_
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
"""

# 8. attribute.h
files['attribute.h'] = """#ifndef CONGELADO_C_PARSER_ATTRIBUTE_H_
#define CONGELADO_C_PARSER_ATTRIBUTE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/tensor.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserAttribute { void* plugin_data; } TFParserAttribute;

    typedef struct TFParserAttributeOps {
        size_t struct_size;
        void (*get_name)(TFParserAttribute* attribute, TF_String* out_name, TF_Status* out_status);
        void (*get_value)(TFParserAttribute* attribute, TF_Tensor** out_value, TF_Status* out_status);
    } TFParserAttributeOps;
    #define TF_PARSER_ATTRIBUTE_STRUCT_SIZE TF_OFFSET_OF_END(TFParserAttributeOps, get_value)

    TF_CAPI_EXPORT void create_parser_attribute(TFParserAttributeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_attribute(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 9. definition.h
files['definition.h'] = """#ifndef CONGELADO_C_PARSER_DEFINITION_H_
#define CONGELADO_C_PARSER_DEFINITION_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserDefinition { void* plugin_data; } TFParserDefinition;

    typedef struct TFParserDefinitionOps {
        size_t struct_size;
        void (*get_name)(TFParserDefinition* definition, TF_String* out_name, TF_Status* out_status);
        void (*get_source_file)(TFParserDefinition* definition, TF_String* out_source_file, TF_Status* out_status);
        void (*get_line_number)(TFParserDefinition* definition, int* out_line_number, TF_Status* out_status);
    } TFParserDefinitionOps;
    #define TF_PARSER_DEFINITION_STRUCT_SIZE TF_OFFSET_OF_END(TFParserDefinitionOps, get_line_number)

    TF_CAPI_EXPORT void create_parser_definition(TFParserDefinitionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_definition(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

# 10. parser.h
files['parser.h'] = """#ifndef CONGELADO_C_PARSER_H_
#define CONGELADO_C_PARSER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/parser/catalog.h"
#include "include/c/extern/parser/module.h"
#include "include/c/extern/parser/function.h"
#include "include/c/extern/parser/parameter.h"
#include "include/c/extern/parser/typeinfo.h"
#include "include/c/extern/parser/attribute.h"
#include "include/c/extern/parser/definition.h"
#include "include/c/extern/parser/block.h"
#include "include/c/extern/parser/node.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Parser {
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

    typedef struct TF_ParserOps {
        size_t struct_size;
        void (*destroy)(TF_Parser* parser);
        void (*get_name)(TF_Parser* parser, TF_String* out_name);
    } TF_ParserOps;
    #define TF_PARSER_STRUCT_SIZE TF_OFFSET_OF_END(TF_ParserOps, get_name)

    TF_CAPI_EXPORT void create_parser(TF_ParserOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser(void* plugin_context);

    static inline void init_parser(TF_ParserOps** ops, TF_Parser* parser, TF_Status* out_status) {
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
"""

for filename, content in files.items():
    with open(os.path.join(parser_dir, filename), 'w') as f:
        f.write(content)
