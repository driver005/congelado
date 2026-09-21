import os

# --- PARSER ---
parser_dir = 'include/c/extern/parser'
os.makedirs(parser_dir, exist_ok=True)

catalog_h = """#ifndef CONGELADO_C_PARSER_CATALOG_H_
#define CONGELADO_C_PARSER_CATALOG_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserCatalog { void* plugin_data; } TFParserCatalog;

    typedef struct TFParserCatalogOps {
        size_t struct_size;
        void (*parse_file)(TFParserCatalog* catalog, const TF_String* file_path, void** out_document_handle, TF_Status* out_status);
        void (*parse_buffer)(TFParserCatalog* catalog, const TF_Buffer* buffer, void** out_document_handle, TF_Status* out_status);
        void (*free_document)(TFParserCatalog* catalog, void* document_handle);
    } TFParserCatalogOps;
    #define TF_PARSER_CATALOG_STRUCT_SIZE TF_OFFSET_OF_END(TFParserCatalogOps, free_document)

    TF_CAPI_EXPORT void create_parser_catalog(TFParserCatalogOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_catalog(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

document_h = """#ifndef CONGELADO_C_PARSER_DOCUMENT_H_
#define CONGELADO_C_PARSER_DOCUMENT_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserDocument { void* plugin_data; } TFParserDocument;

    typedef struct TFParserDocumentOps {
        size_t struct_size;
        void (*get_root_node)(TFParserDocument* doc, void* document_handle, void** out_node_handle, TF_Status* out_status);
        void (*get_format)(TFParserDocument* doc, void* document_handle, TF_String* out_format, TF_Status* out_status);
    } TFParserDocumentOps;
    #define TF_PARSER_DOCUMENT_STRUCT_SIZE TF_OFFSET_OF_END(TFParserDocumentOps, get_format)

    TF_CAPI_EXPORT void create_parser_document(TFParserDocumentOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_document(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

node_h = """#ifndef CONGELADO_C_PARSER_NODE_H_
#define CONGELADO_C_PARSER_NODE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/tensor.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFParserNode { void* plugin_data; } TFParserNode;

    typedef struct TFParserNodeOps {
        size_t struct_size;
        void (*get_name)(TFParserNode* node, void* node_handle, TF_String* out_name, TF_Status* out_status);
        void (*get_op_type)(TFParserNode* node, void* node_handle, TF_String* out_op_type, TF_Status* out_status);
        void (*get_children)(TFParserNode* node, void* node_handle, TF_Tensor** out_children_handles, TF_Status* out_status);
        void (*get_attribute)(TFParserNode* node, void* node_handle, const TF_String* attr_name, TF_Tensor** out_attr_value, TF_Status* out_status);
    } TFParserNodeOps;
    #define TF_PARSER_NODE_STRUCT_SIZE TF_OFFSET_OF_END(TFParserNodeOps, get_attribute)

    TF_CAPI_EXPORT void create_parser_node(TFParserNodeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_node(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

parser_main_h = """#ifndef CONGELADO_C_PARSER_H_
#define CONGELADO_C_PARSER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/parser/catalog.h"
#include "include/c/extern/parser/document.h"
#include "include/c/extern/parser/node.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Parser {
        void* plugin_data;
        const TFParserCatalogOps* catalog_ops;
        const TFParserDocumentOps* document_ops;
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

        TFParserDocumentOps* document_ops = NULL;
        create_parser_document(&document_ops, &parser->plugin_data, out_status);
        parser->document_ops = document_ops;

        TFParserNodeOps* node_ops = NULL;
        create_parser_node(&node_ops, &parser->plugin_data, out_status);
        parser->node_ops = node_ops;
    }

#ifdef __cplusplus
}
#endif
#endif
"""

with open(os.path.join(parser_dir, 'catalog.h'), 'w') as f: f.write(catalog_h)
with open(os.path.join(parser_dir, 'document.h'), 'w') as f: f.write(document_h)
with open(os.path.join(parser_dir, 'node.h'), 'w') as f: f.write(node_h)
with open(os.path.join(parser_dir, 'parser.h'), 'w') as f: f.write(parser_main_h)

# --- GRAPPLER ---
grappler_dir = 'include/c/extern/grappler'
os.makedirs(grappler_dir, exist_ok=True)

optimizer_h = """#ifndef CONGELADO_C_GRAPPLER_OPTIMIZER_H_
#define CONGELADO_C_GRAPPLER_OPTIMIZER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerOptimizer { void* plugin_data; } TFGrapplerOptimizer;

    typedef struct TFGrapplerOptimizerOps {
        size_t struct_size;
        void (*optimize)(TFGrapplerOptimizer* optimizer, const TF_Buffer* graph_buf, TF_Buffer* out_optimized_graph_buf, TF_Status* out_status);
    } TFGrapplerOptimizerOps;
    #define TF_GRAPPLER_OPTIMIZER_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerOptimizerOps, optimize)

    TF_CAPI_EXPORT void create_grappler_optimizer(TFGrapplerOptimizerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_optimizer(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

pass_h = """#ifndef CONGELADO_C_GRAPPLER_PASS_H_
#define CONGELADO_C_GRAPPLER_PASS_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerPass { void* plugin_data; } TFGrapplerPass;

    typedef struct TFGrapplerPassOps {
        size_t struct_size;
        void (*get_name)(TFGrapplerPass* pass, TF_String* out_name, TF_Status* out_status);
        void (*enable_pass)(TFGrapplerPass* pass, int enable, TF_Status* out_status);
    } TFGrapplerPassOps;
    #define TF_GRAPPLER_PASS_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerPassOps, enable_pass)

    TF_CAPI_EXPORT void create_grappler_pass(TFGrapplerPassOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_pass(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

grappler_main_h = """#ifndef CONGELADO_C_GRAPPLER_H_
#define CONGELADO_C_GRAPPLER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/grappler/pass.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Grappler {
        void* plugin_data;
        const TFGrapplerOptimizerOps* optimizer_ops;
        const TFGrapplerPassOps* pass_ops;
    } TF_Grappler;

    typedef struct TF_GrapplerOps {
        size_t struct_size;
        void (*destroy)(TF_Grappler* grappler);
        void (*get_name)(TF_Grappler* grappler, TF_String* out_name);
    } TF_GrapplerOps;
    #define TF_GRAPPLER_STRUCT_SIZE TF_OFFSET_OF_END(TF_GrapplerOps, get_name)

    TF_CAPI_EXPORT void create_grappler(TF_GrapplerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler(void* plugin_context);

    static inline void init_grappler(TF_GrapplerOps** ops, TF_Grappler* grappler, TF_Status* out_status) {
        create_grappler(ops, &grappler->plugin_data, out_status);

        TFGrapplerOptimizerOps* optimizer_ops = NULL;
        create_grappler_optimizer(&optimizer_ops, &grappler->plugin_data, out_status);
        grappler->optimizer_ops = optimizer_ops;

        TFGrapplerPassOps* pass_ops = NULL;
        create_grappler_pass(&pass_ops, &grappler->plugin_data, out_status);
        grappler->pass_ops = pass_ops;
    }

#ifdef __cplusplus
}
#endif
#endif
"""

with open(os.path.join(grappler_dir, 'optimizer.h'), 'w') as f: f.write(optimizer_h)
with open(os.path.join(grappler_dir, 'pass.h'), 'w') as f: f.write(pass_h)
with open(os.path.join(grappler_dir, 'grappler.h'), 'w') as f: f.write(grappler_main_h)
