#ifndef CONGELADO_C_GENERATOR_TYPEINFO_H_
#define CONGELADO_C_GENERATOR_TYPEINFO_H_

#include "c/macros.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_TypeInfo
    {
        void* plugin_data;
    } TF_TypeInfo;

    typedef struct TF_TypeInfoOps
    {
        size_t struct_size;
        void (*destroy)(TF_TypeInfo* type_context);
        void (*get_name)(TF_TypeInfo* type_context, TF_String* out_name);

        void (*set_type_attr_name)(TF_TypeInfo* type_context, const TF_String* type_attr_name);
        void (*set_data_type)(TF_TypeInfo* type_context, int data_type);
        void (*set_read_only)(TF_TypeInfo* type_context, bool read_only);
        void (*set_list)(TF_TypeInfo* type_context, bool is_list);

        void (*get_type_attr_name)(TF_TypeInfo* type_context, TF_String* out_type_attr_name);
        void (*get_data_type)(TF_TypeInfo* type_context, int* out_data_type);

        void (*is_read_only)(TF_TypeInfo* type_context, int* out_is_read_only);
        void (*is_list)(TF_TypeInfo* type_context, int* out_is_list);
    } TF_TypeInfoOps;

#define TF_TYPEINFO_STRUCT_SIZE TF_OFFSET_OF_END(TF_TypeInfoOps, is_list)

    TF_CAPI_EXPORT void
    create_typeinfo(TF_TypeInfoOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_typeinfo(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_TYPEINFO_H_
