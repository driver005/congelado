#ifndef CONGELADO_C_GENERATOR_H_
#define CONGELADO_C_GENERATOR_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_tensor.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*set_name)(void* plugin_context, const TF_String* name);
        void (*get_name)(void* plugin_context, TF_String* out);
        TF_Tensor_Handle* (*get_definitions)(void* plugin_context, TF_Status* status);
        bool (*build)(void* plugin_context, TF_String* out, TF_Status* status);
        void* (*enter_border_patrol)(void* plugin_context, const TF_String* name, TF_Status* status);
        
        // Function
        void (*function__destroy)(void* function_context);
        void* (*function__add_parameter)(void* function_context, const TF_String* name, const TF_String* type_text, TF_Status* status);
        bool (*function__add_node)(
            void* function_context, 
            const void* def_context, 
            const TF_Tensor_Handle* operands, 
            const TF_Tensor_Handle* attrs, 
            TF_Tensor_Handle* out_results, 
            TF_Status* status
        );
        bool (*function__exit_border_patrol)(
            void* function_context, 
            const TF_Tensor_Handle* outputs, 
            TF_Status* status
        );

        // Definition
        void (*definition__destroy)(void* def_context);
        void (*definition__get_name)(void* def_context, TF_String* out);
        void (*definition__get_summary)(void* def_context, TF_String* out);
        void (*definition__get_description)(void* def_context, TF_String* out);
        TF_Tensor_Handle* (*definition__get_inputs)(void* def_context, TF_Status* status);
        TF_Tensor_Handle* (*definition__get_outputs)(void* def_context, TF_Status* status);
        TF_Tensor_Handle* (*definition__get_attrs)(void* def_context, TF_Status* status);

        // Parameter
        void (*parameter__destroy)(void* param_context);
        void (*parameter__get_name)(void* param_context, TF_String* out);
        void (*parameter__get_description)(void* param_context, TF_String* out);
        int (*parameter__get_position)(void* param_context);
        const void* (*parameter__get_type)(void* param_context);

        // Attribute
        void (*attribute__destroy)(void* attr_context);
        void (*attribute__get_name)(void* attr_context, TF_String* out);
        void (*attribute__get_description)(void* attr_context, TF_String* out);
        void (*attribute__get_full_type)(void* attr_context, TF_String* out);
        void (*attribute__get_base_type)(void* attr_context, TF_String* out);
        bool (*attribute__is_list)(void* attr_context);

        // TypeInfo
        void (*typeinfo__destroy)(void* type_context);
        int (*typeinfo__get_data_type)(void* type_context);
        void (*typeinfo__get_type_attr_name)(void* type_context, TF_String* out);
        bool (*typeinfo__is_read_only)(void* type_context);
        bool (*typeinfo__is_list)(void* type_context);
    } TF_Generator;

    TF_CAPI_EXPORT extern void TF_InitGenerator(TF_Generator** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_H_
