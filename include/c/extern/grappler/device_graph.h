#ifndef CONGELADO_C_EXTERN_GRAPPLER_DEVICE_GRAPH_H_
#define CONGELADO_C_EXTERN_GRAPPLER_DEVICE_GRAPH_H_

#include "include/c/macros.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/random_generator/random_generator.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    // A recorded device command graph (capture once, replay many). Distinct from the GraphDef that TFGrapplerOptimizerOps rewrites.
    typedef struct TFGrapplerDeviceGraph { void* plugin_data; } TFGrapplerDeviceGraph;

    typedef enum TF_CaptureMode {
        TF_CAPTURE_MODE_GLOBAL = 0,
        TF_CAPTURE_MODE_THREAD_LOCAL = 1,
        TF_CAPTURE_MODE_RELAXED = 2,
    } TF_CaptureMode;

    // TFGrapplerDeviceGraphOps — created by TF_GrapplerOps::create_device_graph_internal. Replaces XPUGraph (command_graph modifiable/executable).
    typedef struct TFGrapplerDeviceGraphOps {
        size_t struct_size;
        void (*create)(TFGrapplerDeviceGraph* out_handle);
        void (*destroy)(TFGrapplerDeviceGraph* handle);
        void (*capture_begin)(TFGrapplerDeviceGraph* graph, TF_Stream* capture_stream, const TF_PoolId* pool_id, TF_CaptureMode mode, TF_Status* out_status);
        void (*capture_end)(TFGrapplerDeviceGraph* graph, TF_Status* out_status);
        void (*instantiate)(TFGrapplerDeviceGraph* graph, TF_Status* out_status);
        void (*replay)(TFGrapplerDeviceGraph* graph, TF_Status* out_status);
        void (*reset)(TFGrapplerDeviceGraph* graph, TF_Status* out_status);
        void (*get_pool)(TFGrapplerDeviceGraph* graph, TF_PoolId* out_pool_id);
        void (*register_random_generator)(TFGrapplerDeviceGraph* graph, TF_RandomGenerator* generator, TF_Status* out_status);
        void (*unregister_random_generator)(TFGrapplerDeviceGraph* graph, TF_RandomGenerator* generator, TF_Status* out_status);
        void (*enable_debug_mode)(TFGrapplerDeviceGraph* graph);
        void (*debug_dump)(TFGrapplerDeviceGraph* graph, const TF_String* path, TF_Status* out_status);
    } TFGrapplerDeviceGraphOps;
    #define TF_GRAPPLER_DEVICE_GRAPH_STRUCT_SIZE TF_OFFSET_OF_END(TFGrapplerDeviceGraphOps, debug_dump)

    TF_CAPI_EXPORT void create_grappler_device_graph(TFGrapplerDeviceGraphOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_grappler_device_graph(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // CONGELADO_C_EXTERN_GRAPPLER_DEVICE_GRAPH_H_
