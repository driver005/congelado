#ifndef CONGELADO_C_GENERATOR_DEFINITION_H_
#define CONGELADO_C_GENERATOR_DEFINITION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/parameter.h"
#include "c/extern/generator/attribute.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Definition
    {
        void* plugin_data;
    } TF_Generator_Definition;

    typedef struct TF_Generator_DefinitionOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator_Definition* def_context);
        void (*get_name)(TF_Generator_Definition* def_context, TF_String* out_name);

        void (*set_name)(TF_Generator_Definition* def_context, const TF_String* name);
        void (*set_summary)(TF_Generator_Definition* def_context, const TF_String* summary);
        void (*set_description)(TF_Generator_Definition* def_context, const TF_String* description);

        void (*add_input)(
            TF_Generator_Definition* def_context,
            TF_Generator_Parameter* input,
            TF_Status* out_status
        );
        void (*add_output)(
            TF_Generator_Definition* def_context,
            TF_Generator_Parameter* output,
            TF_Status* out_status
        );
        void (*add_attr)(
            TF_Generator_Definition* def_context,
            TF_Generator_Attribute* attr,
            TF_Status* out_status
        );

        void (*get_summary)(TF_Generator_Definition* def_context, TF_String* out_summary);
        void (*get_description)(TF_Generator_Definition* def_context, TF_String* out_description);

        void (*list_inputs)(TF_Generator_Definition* def_context, TF_Tensor** out_inputs, TF_Status* out_status);
        void (*list_outputs)(TF_Generator_Definition* def_context, TF_Tensor** out_outputs, TF_Status* out_status);
        void (*list_attrs)(TF_Generator_Definition* def_context, TF_Tensor** out_attrs, TF_Status* out_status);
    } TF_Generator_DefinitionOps;

#define TF_GENERATOR_DEFINITION_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_DefinitionOps, list_attrs)

    TF_CAPI_EXPORT void
    create_generator_definition(TF_Generator_DefinitionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_definition(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_DEFINITION_H_
