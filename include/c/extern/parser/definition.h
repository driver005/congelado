#ifndef CONGELADO_C_PARSER_DEFINITION_H_
#define CONGELADO_C_PARSER_DEFINITION_H_

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFParserDefinition
    {
        void* plugin_data;
    } TFParserDefinition;

    typedef struct TFParserDefinitionOps
    {
        size_t struct_size;
        void (*create)(TFParserDefinition* out_handle);
        void (*destroy)(TFParserDefinition* handle);
        void (*get_name)(
            TFParserDefinition* definition,
            TF_String* out_name,
            TF_Status* out_status
        );
        void (*get_source_file)(
            TFParserDefinition* definition,
            TF_String* out_source_file,
            TF_Status* out_status
        );
        void (*get_line_number)(
            TFParserDefinition* definition,
            int* out_line_number,
            TF_Status* out_status
        );
    } TFParserDefinitionOps;

#define TF_PARSER_DEFINITION_STRUCT_SIZE TF_OFFSET_OF_END(TFParserDefinitionOps, get_line_number)

    TF_CAPI_EXPORT void create_parser_definition(
        TFParserDefinitionOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_parser_definition(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
