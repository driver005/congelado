#ifndef CONGELADO_C_PARSER_ATTRIBUTE_H_
#define CONGELADO_C_PARSER_ATTRIBUTE_H_

#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFParserAttribute
    {
        void* plugin_data;
    } TFParserAttribute;

    typedef struct TFParserAttributeOps
    {
        size_t struct_size;
        void (*create)(TFParserAttribute* out_handle);
        void (*destroy)(TFParserAttribute* handle);
        void (*get_name)(TFParserAttribute* attribute, TF_String* out_name, TF_Status* out_status);
        void (*get_value)(
            TFParserAttribute* attribute,
            TF_Tensor** out_value,
            TF_Status* out_status
        );
    } TFParserAttributeOps;

#define TF_PARSER_ATTRIBUTE_STRUCT_SIZE TF_OFFSET_OF_END(TFParserAttributeOps, get_value)

    TF_CAPI_EXPORT void create_parser_attribute(
        TFParserAttributeOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_parser_attribute(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
