#ifndef CONGELADO_C_GENERATOR_MODULE_H_
#define CONGELADO_C_GENERATOR_MODULE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/function.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Module
    {
        void* plugin_data;
    } TF_Generator_Module;

    typedef struct TF_Generator_ModuleOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator_Module* module);
        void (*get_name)(TF_Generator_Module* module, TF_String* out_name);

        void (*add_function)(
            TF_Generator_Module* module,
            TF_Generator_Function* function,
            TF_Status* out_status
        );

        void (*get_function)(TF_Generator_Module* module, const TF_String* name, TF_Generator_Function* out_function, TF_Status* out_status);

        void (*list_functions)(TF_Generator_Module* module, TF_Tensor** out_functions, TF_Status* out_status);

        void (*set_name)(TF_Generator_Module* module, const TF_String* name);
        void (*validate)(TF_Generator_Module* module, TF_Status* out_status);
        void (*emit)(TF_Generator_Module* module, TF_String* out_code, TF_Status* out_status);
    } TF_Generator_ModuleOps;

#define TF_GENERATOR_MODULE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_ModuleOps, emit)

    TF_CAPI_EXPORT void
    create_generator_module(TF_Generator_ModuleOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_module(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_MODULE_H_
