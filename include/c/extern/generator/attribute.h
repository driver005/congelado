#ifndef CONGELADO_C_GENERATOR_ATTRIBUTE_H_
#define CONGELADO_C_GENERATOR_ATTRIBUTE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Attribute
    {
        void* plugin_data;
    } TF_Generator_Attribute;

    typedef struct TF_Generator_AttributeOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator_Attribute* attr_context);
        void (*get_name)(TF_Generator_Attribute* attr_context, TF_String* out_name);

        void (*set_name)(TF_Generator_Attribute* attr_context, const TF_String* name);
        void (*set_description)(TF_Generator_Attribute* attr_context, const TF_String* description);
        void (*set_full_type)(TF_Generator_Attribute* attr_context, const TF_String* full_type);
        void (*set_base_type)(TF_Generator_Attribute* attr_context, const TF_String* base_type);
        void (*set_is_list)(TF_Generator_Attribute* attr_context, bool is_list);

        void (*get_description)(TF_Generator_Attribute* attr_context, TF_String* out_description);
        void (*get_full_type)(TF_Generator_Attribute* attr_context, TF_String* out_full_type);
        void (*get_base_type)(TF_Generator_Attribute* attr_context, TF_String* out_base_type);
        void (*is_list)(TF_Generator_Attribute* attr_context, int* out_is_list);
    } TF_Generator_AttributeOps;

#define TF_GENERATOR_ATTRIBUTE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_AttributeOps, is_list)

    TF_CAPI_EXPORT void
    create_generator_attribute(TF_Generator_AttributeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_attribute(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_ATTRIBUTE_H_
