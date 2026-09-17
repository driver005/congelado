#ifndef CONGELADO_C_GENERATOR_H_
#define CONGELADO_C_GENERATOR_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tensor.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/generator/function.h"
#include "c/extern/generator/definition.h"
#include "c/extern/generator/parameter.h"
#include "c/extern/generator/typeinfo.h"
#include "c/extern/generator/attribute.h"
#include "c/extern/generator/module.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator
    {
        void* plugin_data;
        const TF_Generator_ModuleOps* module_ops;
        const TF_Generator_FunctionOps* function_ops;
        const TF_Generator_ParameterOps* parameter_ops;
        const TF_Generator_AttributeOps* attribute_ops;
        const TF_Generator_DefinitionOps* definition_ops;
        const TF_Generator_BlockOps* block_ops;
        const TF_Generator_NodeOps* node_ops;
        const TF_TypeInfoOps* typeinfo_ops;
    } TF_Generator;

    typedef struct TF_GeneratorOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator* generator);
        void (*get_name)(TF_Generator* generator, TF_String* out_name);

        void (*add_module)(
            TF_Generator* generator,
            TF_Generator_Module* module,
            TF_Status* out_status
        );

        void (*get_module)(TF_Generator* generator, const TF_String* name, TF_Generator_Module* out_module, TF_Status* out_status);

        void (*list_modules)(TF_Generator* generator, TF_Tensor** out_modules, TF_Status* out_status);

        void (*set_name)(TF_Generator* generator, const TF_String* name);
    } TF_GeneratorOps;

#define TF_GENERATOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_GeneratorOps, set_name)

    TF_CAPI_EXPORT void
    create_generator(TF_GeneratorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_generator(void* plugin_context);

    // Real implementation, not declared-only like create_generator — calls every generator/*.h create_x and fills in generator's ops fields.
    static inline void init_generator(TF_GeneratorOps** ops, TF_Generator* generator, TF_Status* out_status)
    {
        create_generator(ops, &generator->plugin_data, out_status);

        TF_Generator_ModuleOps* module_ops = NULL;
        create_generator_module(&module_ops, &generator->plugin_data, out_status);
        generator->module_ops = module_ops;

        TF_Generator_FunctionOps* function_ops = NULL;
        create_generator_function(&function_ops, &generator->plugin_data, out_status);
        generator->function_ops = function_ops;

        TF_Generator_ParameterOps* parameter_ops = NULL;
        create_generator_parameter(&parameter_ops, &generator->plugin_data, out_status);
        generator->parameter_ops = parameter_ops;

        TF_Generator_AttributeOps* attribute_ops = NULL;
        create_generator_attribute(&attribute_ops, &generator->plugin_data, out_status);
        generator->attribute_ops = attribute_ops;

        TF_Generator_DefinitionOps* definition_ops = NULL;
        create_generator_definition(&definition_ops, &generator->plugin_data, out_status);
        generator->definition_ops = definition_ops;

        TF_Generator_BlockOps* block_ops = NULL;
        create_generator_block(&block_ops, &generator->plugin_data, out_status);
        generator->block_ops = block_ops;

        TF_Generator_NodeOps* node_ops = NULL;
        create_generator_node(&node_ops, &generator->plugin_data, out_status);
        generator->node_ops = node_ops;

        TF_TypeInfoOps* typeinfo_ops = NULL;
        create_typeinfo(&typeinfo_ops, &generator->plugin_data, out_status);
        generator->typeinfo_ops = typeinfo_ops;
    }

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_H_
