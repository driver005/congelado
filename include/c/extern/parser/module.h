#ifndef CONGELADO_C_PARSER_MODULE_H_
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
