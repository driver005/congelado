#ifndef CONGELADO_C_GENERATOR_FUNCTION_H_
#define CONGELADO_C_GENERATOR_FUNCTION_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/generator/definition.h"
#include "include/c/extern/generator/parameter.h"
#include "include/c/extern/generator/attribute.h"
#include "include/c/extern/generator/block.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFGeneratorFunction
    {
        void* plugin_data;
    } TFGeneratorFunction;

    typedef struct TFGeneratorFunctionOps
    {
        size_t struct_size;
        void (*destroy)(TFGeneratorFunction* function);
        void (*get_name)(TFGeneratorFunction* function, TF_String* out_name);

        void (*add_parameter)(
            TFGeneratorFunction* function,
            TFGeneratorParameter* parameter,
            TF_Status* out_status
        );
        void (*add_attribute)(
            TFGeneratorFunction* function,
            TFGeneratorAttribute* attribute,
            TF_Status* out_status
        );
        void (*add_definition)(
            TFGeneratorFunction* function,
            TFGeneratorDefinition* definition,
            TF_Status* out_status
        );
        void (*add_block)(
            TFGeneratorFunction* function,
            TFGeneratorBlock* block,
            TF_Status* out_status
        );

        void (*get_parameter)(
            TFGeneratorFunction* function,
            const TF_String* name,
            TFGeneratorParameter* out_parameter,
            TF_Status* out_status
        );
        void (*get_attribute)(
            TFGeneratorFunction* function,
            const TF_String* name,
            TFGeneratorAttribute* out_attribute,
            TF_Status* out_status
        );
        void (*get_definition)(TFGeneratorFunction* function, const TF_String* name, TFGeneratorDefinition* out_definition, TF_Status* out_status);
        void (*get_block)(TFGeneratorFunction* function, const TF_String* name, TFGeneratorBlock* out_block, TF_Status* out_status);

        void (*list_parameters)(TFGeneratorFunction* function, TF_Tensor** out_parameters, TF_Status* out_status);
        void (*list_attributes)(TFGeneratorFunction* function, TF_Tensor** out_attributes, TF_Status* out_status);
        void (*list_definitions)(TFGeneratorFunction* function, TF_Tensor** out_definitions, TF_Status* out_status);
        void (*list_blocks)(TFGeneratorFunction* function, TF_Tensor** out_blocks, TF_Status* out_status);

        void (*finish)(
            TFGeneratorFunction* function,
            const TF_Tensor* outputs,
            TF_Status* out_status
        );
    } TFGeneratorFunctionOps;

#define TF_GENERATOR_FUNCTION_STRUCT_SIZE TF_OFFSET_OF_END(TFGeneratorFunctionOps, finish)

    TF_CAPI_EXPORT void
    create_generator_function(TFGeneratorFunctionOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator_function(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_FUNCTION_H_
