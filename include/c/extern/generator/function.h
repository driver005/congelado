#ifndef CONGELADO_C_GENERATOR_FUNCTION_H_
#define CONGELADO_C_GENERATOR_FUNCTION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Definition TF_Generator_Definition;
    typedef struct TF_Generator_Parameter TF_Generator_Parameter;

    // --------------------------------------------------------------------------
    // TF_Generator_Function — in-progress function body being built by a
    // TF_Generator (via its create_function slot).
    typedef struct TF_Generator_Function_Handle TF_Generator_Function_Handle;

    // Plugin-facing vtable registered via init_generator_function.
    typedef struct TF_Generator_Function
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        void (*destroy_function)(TF_Generator_Function_Handle* function);
        TF_Generator_Parameter* (*add_parameter)(
            TF_Generator_Function_Handle* function,
            const TF_String_Handle* name,
            const TF_String_Handle* type_text,
            TF_Status_Handle* status
        );
        void (*add_node)(
            TF_Generator_Function_Handle* function,
            const TF_Generator_Definition* def_context,
            const TF_Tensor_Handle* operands,
            const TF_Tensor_Handle* attrs,
            TF_Tensor_Handle* out_results,
            TF_Status_Handle* status
        );
        void (*finish)(
            TF_Generator_Function_Handle* function,
            const TF_Tensor_Handle* outputs,
            TF_Status_Handle* status
        );
    } TF_Generator_Function;

#define TF_GENERATOR_FUNCTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_Function, finish)

    TF_CAPI_EXPORT void
    init_generator_function(TF_Generator_Function** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_FUNCTION_H_
