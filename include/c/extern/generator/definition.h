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

    typedef struct TFGeneratorDefinition
    {
        void* plugin_data;
    } TFGeneratorDefinition;

    typedef struct TFGeneratorDefinitionOps
    {
        size_t struct_size;
        void (*destroy)(TFGeneratorDefinition* def_context);
        void (*get_name)(TFGeneratorDefinition* def_context, TF_String* out_name);

        void (*set_name)(TFGeneratorDefinition* def_context, const TF_String* name);
        void (*set_summary)(TFGeneratorDefinition* def_context, const TF_String* summary);
        void (*set_description)(TFGeneratorDefinition* def_context, const TF_String* description);

        void (*add_input)(
            TFGeneratorDefinition* def_context,
            TFGeneratorParameter* input,
            TF_Status* out_status
        );
        void (*add_output)(
            TFGeneratorDefinition* def_context,
            TFGeneratorParameter* output,
            TF_Status* out_status
        );
        void (*add_attr)(
            TFGeneratorDefinition* def_context,
            TFGeneratorAttribute* attr,
            TF_Status* out_status
        );

        void (*get_summary)(TFGeneratorDefinition* def_context, TF_String* out_summary);
        void (*get_description)(TFGeneratorDefinition* def_context, TF_String* out_description);

        void (*list_inputs)(TFGeneratorDefinition* def_context, TF_Tensor** out_inputs, TF_Status* out_status);
        void (*list_outputs)(TFGeneratorDefinition* def_context, TF_Tensor** out_outputs, TF_Status* out_status);
        void (*list_attrs)(TFGeneratorDefinition* def_context, TF_Tensor** out_attrs, TF_Status* out_status);
    } TFGeneratorDefinitionOps;

#define TF_GENERATOR_DEFINITION_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorDefinitionOps, list_attrs)

    TF_CAPI_EXPORT void
    create_generator_definition(TFGeneratorDefinitionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_definition(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_DEFINITION_H_
