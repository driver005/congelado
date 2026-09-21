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
