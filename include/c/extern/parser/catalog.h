#ifndef CONGELADO_C_PARSER_CATALOG_H_
#define CONGELADO_C_PARSER_CATALOG_H_

#include "include/c/extern/parser/module.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFParserCatalog
    {
        void* plugin_data;
    } TFParserCatalog;

    typedef struct TFParserCatalogOps
    {
        size_t struct_size;
        void (*create)(TFParserCatalog* out_handle);
        void (*destroy)(TFParserCatalog* handle);
        void (*parse_file)(
            TFParserCatalog* catalog,
            const TF_String* file_path,
            TFParserModule* out_module,
            TF_Status* out_status
        );
        void (*parse_buffer)(
            TFParserCatalog* catalog,
            const TF_Buffer* buffer,
            TFParserModule* out_module,
            TF_Status* out_status
        );
    } TFParserCatalogOps;

#define TF_PARSER_CATALOG_STRUCT_SIZE TF_OFFSET_OF_END(TFParserCatalogOps, parse_buffer)

    TF_CAPI_EXPORT void
    create_parser_catalog(TFParserCatalogOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_catalog(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
