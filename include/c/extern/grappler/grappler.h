#ifndef CONGELADO_C_GRAPPLER_H_
#define CONGELADO_C_GRAPPLER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/grappler/item.h"
#include "include/c/extern/grappler/properties.h"
#include "include/c/extern/grappler/function_library.h"
#include "include/c/extern/grappler/optimizer.h"
#include "include/c/extern/grappler/configs.h"
#include "include/c/extern/grappler/device_graph.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/device.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Grappler {
        void* plugin_data;
        void* item_context;
        void* properties_context;
        void* function_library_context;
        void* optimizer_context;
        void* configs_context;
        void* device_graph_context;
        const TFGrapplerItemOps* item_ops;
        const TFGrapplerPropertiesOps* properties_ops;
        const TFGrapplerFunctionLibraryOps* function_library_ops;
        const TFGrapplerOptimizerOps* optimizer_ops;
        const TFGrapplerConfigsOps* configs_ops;
        const TFGrapplerDeviceGraphOps* device_graph_ops;
    } TF_Grappler;

    typedef struct TF_GrapplerOps {
        size_t struct_size;
        void (*destroy)(TF_Grappler* grappler);
        void (*get_name)(TF_Grappler* grappler, TF_String* out_name);
        void (*create_device_graph_internal)(TF_Grappler* grappler, TF_Executor* executor, TF_Device* device, TFGrapplerDeviceGraph* out_graph, TF_Status* out_status);
        void (*destroy_device_graph_internal)(TF_Grappler* grappler, TFGrapplerDeviceGraph* graph);
    } TF_GrapplerOps;
    #define TF_GRAPPLER_STRUCT_SIZE TF_OFFSET_OF_END(TF_GrapplerOps, destroy_device_graph_internal)

    TF_CAPI_EXPORT void create_grappler(TF_GrapplerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler(void* plugin_context);

    // Each vtable gets its own plugin context slot so create_* calls do not overwrite one another.
    static inline void init_grappler(TF_GrapplerOps** ops, TF_Grappler* grappler, TF_Status* out_status) {
        create_grappler(ops, &grappler->plugin_data, out_status);

        TFGrapplerItemOps* item_ops = NULL;
        create_grappler_item(&item_ops, &grappler->item_context, out_status);
        grappler->item_ops = item_ops;

        TFGrapplerPropertiesOps* properties_ops = NULL;
        create_grappler_properties(&properties_ops, &grappler->properties_context, out_status);
        grappler->properties_ops = properties_ops;

        TFGrapplerFunctionLibraryOps* function_library_ops = NULL;
        create_grappler_function_library(&function_library_ops, &grappler->function_library_context, out_status);
        grappler->function_library_ops = function_library_ops;

        TFGrapplerOptimizerOps* optimizer_ops = NULL;
        create_grappler_optimizer(&optimizer_ops, &grappler->optimizer_context, out_status);
        grappler->optimizer_ops = optimizer_ops;

        TFGrapplerConfigsOps* configs_ops = NULL;
        create_grappler_configs(&configs_ops, &grappler->configs_context, out_status);
        grappler->configs_ops = configs_ops;

        TFGrapplerDeviceGraphOps* device_graph_ops = NULL;
        create_grappler_device_graph(&device_graph_ops, &grappler->device_graph_context, out_status);
        grappler->device_graph_ops = device_graph_ops;
    }

#ifdef __cplusplus
}
#endif
#endif
