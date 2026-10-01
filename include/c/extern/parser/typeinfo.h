#ifndef CONGELADO_C_PARSER_TYPEINFO_H_
#define CONGELADO_C_PARSER_TYPEINFO_H_

#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFParserTypeInfo
    {
        void* plugin_data;
    } TFParserTypeInfo;

    typedef struct TFParserTypeInfoOps
    {
        size_t struct_size;
        void (*create)(TFParserTypeInfo* out_handle);
        void (*destroy)(TFParserTypeInfo* handle);
        void (*get_dtype)(TFParserTypeInfo* typeinfo, int* out_dtype, TF_Status* out_status);
        void (*get_shape)(
            TFParserTypeInfo* typeinfo,
            int64_t** out_dims,
            int* out_num_dims,
            TF_Status* out_status
        );
    } TFParserTypeInfoOps;

#define TF_PARSER_TYPEINFO_STRUCT_SIZE TF_OFFSET_OF_END(TFParserTypeInfoOps, get_shape)

    TF_CAPI_EXPORT void
    create_parser_typeinfo(TFParserTypeInfoOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_parser_typeinfo(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
