#ifndef CONGELADO_C_GRAPPLER_OPTIMIZER_H_
#define CONGELADO_C_GRAPPLER_OPTIMIZER_H_

#include "include/c/extern/grappler/item.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFGrapplerOptimizer
    {
        void* plugin_data;
    } TFGrapplerOptimizer;

    typedef struct TFGrapplerOptimizerOps
    {
        size_t struct_size;
        void (*create)(TFGrapplerOptimizer* out_handle);
        void (*destroy)(TFGrapplerOptimizer* handle);
        void (*optimize)(
            TFGrapplerOptimizer* optimizer,
            const TF_Buffer* graph_buf,
            const TFGrapplerItem* item,
            TF_Buffer* out_optimized_graph_buf,
            TF_Status* out_status
        );
    } TFGrapplerOptimizerOps;

#define TF_GRAPPLER_OPTIMIZER_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerOptimizerOps, optimize)

    TF_CAPI_EXPORT void create_grappler_optimizer(
        TFGrapplerOptimizerOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_grappler_optimizer(void* plugin_context);

#ifdef __cplusplus
}
#endif
#endif
