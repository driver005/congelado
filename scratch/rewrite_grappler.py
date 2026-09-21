import os

grappler_dir = 'include/c/extern/grappler'

item_h = """#ifndef CONGELADO_C_GRAPPLER_ITEM_H_
#define CONGELADO_C_GRAPPLER_ITEM_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerItem { void* plugin_data; } TFGrapplerItem;

    typedef struct TFGrapplerItemOps {
        size_t struct_size;
        void (*get_nodes_to_preserve_size)(TFGrapplerItem* item, int* out_num_values, size_t* out_storage_size, TF_Status* out_status);
        void (*get_nodes_to_preserve_list)(TFGrapplerItem* item, char** out_values, size_t* out_lengths, int num_values, void* storage, size_t storage_size, TF_Status* out_status);
        void (*get_fetch_nodes_size)(TFGrapplerItem* item, int* out_num_values, size_t* out_storage_size, TF_Status* out_status);
        void (*get_fetch_nodes_list)(TFGrapplerItem* item, char** out_values, size_t* out_lengths, int num_values, void* storage, size_t storage_size, TF_Status* out_status);
    } TFGrapplerItemOps;
    #define TF_GRAPPLER_ITEM_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerItemOps, get_fetch_nodes_list)

    TF_CAPI_EXPORT void create_grappler_item(TFGrapplerItemOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_item(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

properties_h = """#ifndef CONGELADO_C_GRAPPLER_PROPERTIES_H_
#define CONGELADO_C_GRAPPLER_PROPERTIES_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/buffer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerProperties { void* plugin_data; } TFGrapplerProperties;

    typedef struct TFGrapplerPropertiesOps {
        size_t struct_size;
        void (*infer_statically)(TFGrapplerProperties* props, bool assume_valid_feeds, bool aggressive_shape_inference, bool include_input_tensor_values, bool include_output_tensor_values, TF_Status* out_status);
        void (*get_input_properties_size)(TFGrapplerProperties* props, const char* name, int* out_num_values, TF_Status* out_status);
        void (*get_output_properties_size)(TFGrapplerProperties* props, const char* name, int* out_num_values, TF_Status* out_status);
        void (*get_input_properties)(TFGrapplerProperties* props, const char* name, TF_Buffer** out_properties, int num_values, TF_Status* out_status);
        void (*get_output_properties)(TFGrapplerProperties* props, const char* name, TF_Buffer** out_properties, int num_values, TF_Status* out_status);
    } TFGrapplerPropertiesOps;
    #define TF_GRAPPLER_PROPERTIES_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerPropertiesOps, get_output_properties)

    TF_CAPI_EXPORT void create_grappler_properties(TFGrapplerPropertiesOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_properties(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

function_library_h = """#ifndef CONGELADO_C_GRAPPLER_FUNCTION_LIBRARY_H_
#define CONGELADO_C_GRAPPLER_FUNCTION_LIBRARY_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerFunctionLibrary { void* plugin_data; } TFGrapplerFunctionLibrary;

    typedef struct TFGrapplerFunctionLibraryOps {
        size_t struct_size;
        void (*look_up_op_def)(TFGrapplerFunctionLibrary* lib, const char* name, TF_Buffer* out_buf, TF_Status* out_status);
    } TFGrapplerFunctionLibraryOps;
    #define TF_GRAPPLER_FUNCTION_LIBRARY_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerFunctionLibraryOps, look_up_op_def)

    TF_CAPI_EXPORT void create_grappler_function_library(TFGrapplerFunctionLibraryOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_function_library(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

optimizer_h = """#ifndef CONGELADO_C_GRAPPLER_OPTIMIZER_H_
#define CONGELADO_C_GRAPPLER_OPTIMIZER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/buffer.h"
#include "include/c/extern/grappler/item.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TFGrapplerOptimizer { void* plugin_data; } TFGrapplerOptimizer;

    typedef struct TFGrapplerOptimizerOps {
        size_t struct_size;
        void (*optimize)(TFGrapplerOptimizer* optimizer, const TF_Buffer* graph_buf, const TFGrapplerItem* item, TF_Buffer* out_optimized_graph_buf, TF_Status* out_status);
    } TFGrapplerOptimizerOps;
    #define TF_GRAPPLER_OPTIMIZER_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerOptimizerOps, optimize)

    TF_CAPI_EXPORT void create_grappler_optimizer(TFGrapplerOptimizerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_optimizer(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

configs_h = """#ifndef CONGELADO_C_GRAPPLER_CONFIGS_H_
#define CONGELADO_C_GRAPPLER_CONFIGS_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef enum TFGrapplerOptimizationLevel {
        TF_GRAPPLER_OPT_L0 = 0,
        TF_GRAPPLER_OPT_L1 = 1,
        TF_GRAPPLER_OPT_L2 = 2,
        TF_GRAPPLER_OPT_L3 = 3
    } TFGrapplerOptimizationLevel;

    typedef struct TFGrapplerConfigs { void* plugin_data; } TFGrapplerConfigs;

    typedef struct TFGrapplerConfigsOps {
        size_t struct_size;
        void (*get_optimization_level)(TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel* out_level);
        void (*set_optimization_level)(TFGrapplerConfigs* configs, TFGrapplerOptimizationLevel level);
    } TFGrapplerConfigsOps;
    #define TF_GRAPPLER_CONFIGS_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerConfigsOps, set_optimization_level)

    TF_CAPI_EXPORT void create_grappler_configs(TFGrapplerConfigsOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_configs(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
"""

grappler_main_h = """#ifndef CONGELADO_C_GRAPPLER_H_
#define CONGELADO_C_GRAPPLER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/properties.h"
#include "include/c/extern/grappler/function_library.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/grappler/configs.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Grappler {
        void* plugin_data;
        const TFGrapplerItemOps* item_ops;
        const TFGrapplerPropertiesOps* properties_ops;
        const TFGrapplerFunctionLibraryOps* function_library_ops;
        const TFGrapplerOptimizerOps* optimizer_ops;
        const TFGrapplerConfigsOps* configs_ops;
    } TF_Grappler;

    typedef struct TF_GrapplerOps {
        size_t struct_size;
        void (*destroy)(TF_Grappler* grappler);
        void (*get_name)(TF_Grappler* grappler, TF_String* out_name);
    } TF_GrapplerOps;
    #define TF_GRAPPLER_STRUCT_SIZE TF_OFFSET_OF_END(TF_GrapplerOps, get_name)

    TF_CAPI_EXPORT void create_grappler(TF_GrapplerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler(void* plugin_context);

    static inline void init_grappler(TF_GrapplerOps** ops, TF_Grappler* grappler, TF_Status* out_status) {
        create_grappler(ops, &grappler->plugin_data, out_status);

        TFGrapplerItemOps* item_ops = NULL;
        create_grappler_item(&item_ops, &grappler->plugin_data, out_status);
        grappler->item_ops = item_ops;

        TFGrapplerPropertiesOps* properties_ops = NULL;
        create_grappler_properties(&properties_ops, &grappler->plugin_data, out_status);
        grappler->properties_ops = properties_ops;

        TFGrapplerFunctionLibraryOps* function_library_ops = NULL;
        create_grappler_function_library(&function_library_ops, &grappler->plugin_data, out_status);
        grappler->function_library_ops = function_library_ops;

        TFGrapplerOptimizerOps* optimizer_ops = NULL;
        create_grappler_optimizer(&optimizer_ops, &grappler->plugin_data, out_status);
        grappler->optimizer_ops = optimizer_ops;

        TFGrapplerConfigsOps* configs_ops = NULL;
        create_grappler_configs(&configs_ops, &grappler->plugin_data, out_status);
        grappler->configs_ops = configs_ops;
    }

#ifdef __cplusplus
}
#endif
#endif
"""

with open(os.path.join(grappler_dir, 'item.h'), 'w') as f: f.write(item_h)
with open(os.path.join(grappler_dir, 'properties.h'), 'w') as f: f.write(properties_h)
with open(os.path.join(grappler_dir, 'function_library.h'), 'w') as f: f.write(function_library_h)
with open(os.path.join(grappler_dir, 'optimizer.h'), 'w') as f: f.write(optimizer_h)
with open(os.path.join(grappler_dir, 'configs.h'), 'w') as f: f.write(configs_h)
with open(os.path.join(grappler_dir, 'grappler.h'), 'w') as f: f.write(grappler_main_h)

# Remove old pass.h
old_pass_h = os.path.join(grappler_dir, 'pass.h')
if os.path.exists(old_pass_h):
    os.remove(old_pass_h)
