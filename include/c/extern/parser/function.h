#ifndef CONGELADO_C_PARSER_FUNCTION_H_
#define CONGELADO_C_PARSER_FUNCTION_H_

#include "include/c/extern/parser/block.h"
#include "include/c/extern/parser/parameter.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFParserFunction
    {
        void* plugin_data;
    } TFParserFunction;

    typedef struct TFParserFunctionOps
    {
        size_t struct_size;
        void (*create)(TFParserFunction* out_handle);
        void (*destroy)(TFParserFunction* handle);
        void (*get_name)(TFParserFunction* function, TF_String* out_name, TF_Status* out_status);
        void (*get_parameter_count)(
            TFParserFunction* function,
            int* out_count,
            TF_Status* out_status
        );
        void (*get_parameter)(
            TFParserFunction* function,
            int index,
            TFParserParameter* out_parameter,
            TF_Status* out_status
        );
        void (*get_block_count)(TFParserFunction* function, int* out_count, TF_Status* out_status);
        void (*get_block)(
            TFParserFunction* function,
            int index,
            TFParserBlock* out_block,
            TF_Status* out_status
        );
    } TFParserFunctionOps;

#define TF_PARSER_FUNCTION_STRUCT_SIZE TF_OFFSET_OF_END(TFParserFunctionOps, get_block)

    TF_CAPI_EXPORT void
    create_parser_function(TFParserFunctionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_function(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
