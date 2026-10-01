#ifndef CONGELADO_C_PARSER_PARAMETER_H_
#define CONGELADO_C_PARSER_PARAMETER_H_

#include "include/c/extern/parser/typeinfo.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFParserParameter
    {
        void* plugin_data;
    } TFParserParameter;

    typedef struct TFParserParameterOps
    {
        size_t struct_size;
        void (*create)(TFParserParameter* out_handle);
        void (*destroy)(TFParserParameter* handle);
        void (*get_name)(TFParserParameter* parameter, TF_String* out_name, TF_Status* out_status);
        void (*get_typeinfo)(
            TFParserParameter* parameter,
            TFParserTypeInfo* out_typeinfo,
            TF_Status* out_status
        );
    } TFParserParameterOps;

#define TF_PARSER_PARAMETER_STRUCT_SIZE TF_OFFSET_OF_END(TFParserParameterOps, get_typeinfo)

    TF_CAPI_EXPORT void create_parser_parameter(
        TFParserParameterOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_parser_parameter(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
