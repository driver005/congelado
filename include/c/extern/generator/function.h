#ifndef CONGELADO_C_GENERATOR_FUNCTION_H_
#define CONGELADO_C_GENERATOR_FUNCTION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/definition.h"
#include "c/extern/generator/parameter.h"
#include "c/extern/generator/attribute.h"
#include "c/extern/generator/block.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Function
    {
        void* plugin_data;
    } TF_Generator_Function;

    typedef struct TF_Generator_FunctionOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator_Function* function);
        void (*get_name)(TF_Generator_Function* function, TF_String* out);

        void (*add_parameter)(
            TF_Generator_Function* function,
            TF_Generator_Parameter* parameter,
            TF_Status* status
        );
        void (*add_attribute)(
            TF_Generator_Function* function,
            TF_Generator_Attribute* attribute,
            TF_Status* status
        );
        void (*add_definition)(
            TF_Generator_Function* function,
            TF_Generator_Definition* definition,
            TF_Status* status
        );
        void (*add_block)(
            TF_Generator_Function* function,
            TF_Generator_Block* block,
            TF_Status* status
        );

        TF_Generator_Parameter* (*get_parameter)(
            TF_Generator_Function* function,
            const TF_String* name
        );
        TF_Generator_Attribute* (*get_attribute)(
            TF_Generator_Function* function,
            const TF_String* name
        );
        TF_Generator_Definition* (*get_definition)(TF_Generator_Function* function, const TF_String* name);
        TF_Generator_Block* (*get_block)(TF_Generator_Function* function, const TF_String* name);

        TF_Tensor* (*list_parameters)(TF_Generator_Function* function, TF_Status* status);
        TF_Tensor* (*list_attributes)(TF_Generator_Function* function, TF_Status* status);
        TF_Tensor* (*list_definitions)(TF_Generator_Function* function, TF_Status* status);
        TF_Tensor* (*list_blocks)(TF_Generator_Function* function, TF_Status* status);

        void (*finish)(
            TF_Generator_Function* function,
            const TF_Tensor* outputs,
            TF_Status* status
        );
    } TF_Generator_FunctionOps;

#define TF_GENERATOR_FUNCTION_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator_FunctionOps, finish)

    TF_CAPI_EXPORT void
    create_generator_function(TF_Generator_FunctionOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_generator_function(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_GENERATOR_FUNCTION_H_
