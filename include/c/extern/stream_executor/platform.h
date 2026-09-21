#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_PLATFORM_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_PLATFORM_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/executor.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Platform { void* plugin_data; } TF_Platform;

    // TF_PlatformOps
    typedef struct TF_PlatformOps {
        size_t struct_size;
        void (*get_device_count)(TF_Platform* platform, int* out_device_count, TF_Status* out_status);
        void (*create_device_internal)(TF_Platform* platform, TF_Device* device, TF_Status* out_status);
        void (*destroy_device_internal)(TF_Platform* platform, TF_Device* device);
        void (*create_executor_internal)(TF_Platform* platform, TF_Executor* executor, TF_Status* out_status);
        void (*destroy_executor_internal)(TF_Platform* platform, TF_Executor* executor);
    } TF_PlatformOps;
    #define TF_PLATFORM_STRUCT_SIZE TF_OFFSET_OF_END(TF_PlatformOps, destroy_executor_internal)
    
    TF_CAPI_EXPORT void create_platform(TF_PlatformOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_platform(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_PLATFORM_H_
