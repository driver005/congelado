#ifndef CONGELADO_C_GENERATOR_PARAMETER_H_
#define CONGELADO_C_GENERATOR_PARAMETER_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/typeinfo.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Parameter
    {
        void* plugin_data;
    } TF_Generator_Parameter;

    typedef struct TF_Generator_ParameterOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator_Parameter* param_context);
        void (*get_name)(TF_Generator_Parameter* param_context, TF_String* out_name);

        void (*set_name)(TF_Generator_Parameter* param_context, const TF_String* name);
        void (*set_description)(TF_Generator_Parameter* param_context, const TF_String* description);
        void (*set_position)(TF_Generator_Parameter* param_context, int position);

        void (*get_description)(TF_Generator_Parameter* param_context, TF_String* out_description);
        void (*get_position)(TF_Generator_Parameter* param_context, int* out_position);
        void (*get_type)(TF_Generator_Parameter* param_context, TF_TypeInfo* out_type);
    } TF_Generator_ParameterOps;

#define TF_GENERATOR_PARAMETER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_ParameterOps, get_type)

    TF_CAPI_EXPORT void
    create_generator_parameter(TF_Generator_ParameterOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_parameter(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_PARAMETER_H_
