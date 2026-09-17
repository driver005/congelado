#ifndef CONGELADO_C_GENERATOR_ATTRIBUTE_H_
#define CONGELADO_C_GENERATOR_ATTRIBUTE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFGeneratorAttribute
    {
        void* plugin_data;
    } TFGeneratorAttribute;

    typedef struct TFGeneratorAttributeOps
    {
        size_t struct_size;
        void (*destroy)(TFGeneratorAttribute* attr_context);
        void (*get_name)(TFGeneratorAttribute* attr_context, TF_String* out_name);

        void (*set_name)(TFGeneratorAttribute* attr_context, const TF_String* name);
        void (*set_description)(TFGeneratorAttribute* attr_context, const TF_String* description);
        void (*set_full_type)(TFGeneratorAttribute* attr_context, const TF_String* full_type);
        void (*set_base_type)(TFGeneratorAttribute* attr_context, const TF_String* base_type);
        void (*set_is_list)(TFGeneratorAttribute* attr_context, bool is_list);

        void (*get_description)(TFGeneratorAttribute* attr_context, TF_String* out_description);
        void (*get_full_type)(TFGeneratorAttribute* attr_context, TF_String* out_full_type);
        void (*get_base_type)(TFGeneratorAttribute* attr_context, TF_String* out_base_type);
        void (*is_list)(TFGeneratorAttribute* attr_context, int* out_is_list);
    } TFGeneratorAttributeOps;

#define TF_GENERATOR_ATTRIBUTE_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorAttributeOps, is_list)

    TF_CAPI_EXPORT void
    create_generator_attribute(TFGeneratorAttributeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_attribute(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_ATTRIBUTE_H_
