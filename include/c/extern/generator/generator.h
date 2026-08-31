#ifndef CONGELADO_C_GENERATOR_H_
#define CONGELADO_C_GENERATOR_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_TypeInfo TF_TypeInfo;
    typedef struct TF_Generator_Function TF_Generator_Function;
    typedef struct TF_Generator_Definition TF_Generator_Definition;
    typedef struct TF_Generator_Parameter TF_Generator_Parameter;
    typedef struct TF_Generator_Attribute TF_Generator_Attribute;

    typedef struct TF_Generator
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*set_name)(void* plugin_context, const TF_TString* name);
        // Returns a plugin-allocated 1-D tensor whose elements are the opaque
        // definition handles consumed by the definition_* slots. Ownership of the
        // returned handle transfers to the caller; release it with the tensor
        // runtime's delete (TF_DeleteTensor via the TF_Tensor ops).
        TF_Tensor_Handle* (*get_definitions)(void* plugin_context, TF_Status* status);
        void (*build)(void* plugin_context, TF_String* out, TF_Status* status);
        TF_Generator_Function* (*create_function)(
            void* plugin_context,
            const TF_TString* name,
            TF_Status* status
        );

        // Function
        void (*function_destroy)(TF_Generator_Function* function_context);
        TF_Generator_Parameter* (*function_add_parameter)(
            TF_Generator_Function* function_context,
            const TF_TString* name,
            const TF_TString* type_text,
            TF_Status* status
        );
        void (*function_add_node)(
            TF_Generator_Function* function_context,
            const TF_Generator_Definition* def_context,
            const TF_Tensor_Handle* operands,
            const TF_Tensor_Handle* attrs,
            TF_Tensor_Handle* out_results,
            TF_Status* status
        );
        void (*function_finish)(
            TF_Generator_Function* function_context,
            const TF_Tensor_Handle* outputs,
            TF_Status* status
        );

        // Definition
        void (*definition_destroy)(TF_Generator_Definition* def_context);
        void (*definition_get_name)(TF_Generator_Definition* def_context, TF_String* out);
        void (*definition_get_summary)(TF_Generator_Definition* def_context, TF_String* out);
        void (*definition_get_description)(TF_Generator_Definition* def_context, TF_String* out);
        TF_Tensor_Handle* (*definition_get_inputs)(TF_Generator_Definition* def_context, TF_Status* status);
        TF_Tensor_Handle* (*definition_get_outputs)(TF_Generator_Definition* def_context, TF_Status* status);
        TF_Tensor_Handle* (*definition_get_attrs)(TF_Generator_Definition* def_context, TF_Status* status);

        // Parameter
        void (*parameter_destroy)(TF_Generator_Parameter* param_context);
        void (*parameter_get_name)(TF_Generator_Parameter* param_context, TF_String* out);
        void (*parameter_get_description)(TF_Generator_Parameter* param_context, TF_String* out);
        int (*parameter_get_position)(TF_Generator_Parameter* param_context);
        // Ownership of the returned type handle transfers to the caller; release
        // it with typeinfo_destroy.
        TF_TypeInfo* (*parameter_get_type)(TF_Generator_Parameter* param_context);

        // Attribute
        void (*attribute_destroy)(TF_Generator_Attribute* attr_context);
        void (*attribute_get_name)(TF_Generator_Attribute* attr_context, TF_String* out);
        void (*attribute_get_description)(TF_Generator_Attribute* attr_context, TF_String* out);
        void (*attribute_get_full_type)(TF_Generator_Attribute* attr_context, TF_String* out);
        void (*attribute_get_base_type)(TF_Generator_Attribute* attr_context, TF_String* out);
        bool (*attribute_is_list)(TF_Generator_Attribute* attr_context);

        // TypeInfo
        void (*typeinfo_destroy)(TF_TypeInfo* type_context);
        int (*typeinfo_get_data_type)(TF_TypeInfo* type_context);
        void (*typeinfo_get_type_attr_name)(TF_TypeInfo* type_context, TF_String* out);
        bool (*typeinfo_is_read_only)(TF_TypeInfo* type_context);
        bool (*typeinfo_is_list)(TF_TypeInfo* type_context);
    } TF_Generator;

#define TF_GENERATOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_Generator, typeinfo_is_list)

    TF_CAPI_EXPORT void
    init_generator(TF_Generator** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_H_
