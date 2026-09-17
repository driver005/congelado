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

    typedef struct TFGeneratorModule
    {
        void* plugin_data;
    } TFGeneratorModule;

    typedef struct TFGeneratorModuleOps
    {
        size_t struct_size;
        void (*destroy)(TFGeneratorModule* module);
        void (*get_name)(TFGeneratorModule* module, TF_String* out_name);

        void (*add_function)(
            TFGeneratorModule* module,
            TFGeneratorFunction* function,
            TF_Status* out_status
        );

        void (*get_function)(TFGeneratorModule* module, const TF_String* name, TFGeneratorFunction* out_function, TF_Status* out_status);

        void (*list_functions)(TFGeneratorModule* module, TF_Tensor** out_functions, TF_Status* out_status);

        void (*set_name)(TFGeneratorModule* module, const TF_String* name);
        void (*validate)(TFGeneratorModule* module, TF_Status* out_status);
        void (*emit)(TFGeneratorModule* module, TF_String* out_code, TF_Status* out_status);
    } TFGeneratorModuleOps;

#define TF_GENERATOR_MODULE_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorModuleOps, emit)

    TF_CAPI_EXPORT void
    create_generator_module(TFGeneratorModuleOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_module(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_MODULE_H_
