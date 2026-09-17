#ifndef CONGELADO_C_GENERATOR_H_
#define CONGELADO_C_GENERATOR_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tensor.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/generator/function.h"
#include "include/c/extern/generator/definition.h"
#include "include/c/extern/generator/parameter.h"
#include "include/c/extern/generator/typeinfo.h"
#include "include/c/extern/generator/attribute.h"
#include "include/c/extern/generator/module.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator
    {
        void* plugin_data;
        const TFGeneratorModuleOps* module_ops;
        const TFGeneratorFunctionOps* function_ops;
        const TFGeneratorParameterOps* parameter_ops;
        const TFGeneratorAttributeOps* attribute_ops;
        const TFGeneratorDefinitionOps* definition_ops;
        const TFGeneratorBlockOps* block_ops;
        const TFGeneratorNodeOps* node_ops;
        const TF_TypeInfoOps* typeinfo_ops;
    } TF_Generator;

    typedef struct TF_GeneratorOps
    {
        size_t struct_size;
        void (*destroy)(TF_Generator* generator);
        void (*get_name)(TF_Generator* generator, TF_String* out_name);

        void (*add_module)(
            TF_Generator* generator,
            TFGeneratorModule* module,
            TF_Status* out_status
        );

        void (*get_module)(TF_Generator* generator, const TF_String* name, TFGeneratorModule* out_module, TF_Status* out_status);

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

        TFGeneratorModuleOps* module_ops = NULL;
        create_generator_module(&module_ops, &generator->plugin_data, out_status);
        generator->module_ops = module_ops;

        TFGeneratorFunctionOps* function_ops = NULL;
        create_generator_function(&function_ops, &generator->plugin_data, out_status);
        generator->function_ops = function_ops;

        TFGeneratorParameterOps* parameter_ops = NULL;
        create_generator_parameter(&parameter_ops, &generator->plugin_data, out_status);
        generator->parameter_ops = parameter_ops;

        TFGeneratorAttributeOps* attribute_ops = NULL;
        create_generator_attribute(&attribute_ops, &generator->plugin_data, out_status);
        generator->attribute_ops = attribute_ops;

        TFGeneratorDefinitionOps* definition_ops = NULL;
        create_generator_definition(&definition_ops, &generator->plugin_data, out_status);
        generator->definition_ops = definition_ops;

        TFGeneratorBlockOps* block_ops = NULL;
        create_generator_block(&block_ops, &generator->plugin_data, out_status);
        generator->block_ops = block_ops;

        TFGeneratorNodeOps* node_ops = NULL;
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
