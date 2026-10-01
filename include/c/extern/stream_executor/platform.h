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
        void (*create)(TF_Platform* out_handle);
        void (*destroy)(TF_Platform* handle);
        void (*get_device_count)(TF_Platform* platform, int* out_device_count, TF_Status* out_status);
        void (*create_device_internal)(TF_Platform* platform, TF_Device* device, TF_Status* out_status);
        void (*destroy_device_internal)(TF_Platform* platform, TF_Device* device);
        void (*create_executor_internal)(TF_Platform* platform, TF_Executor* executor, TF_Status* out_status);
        void (*destroy_executor_internal)(TF_Platform* platform, TF_Executor* executor);
        // current device is per calling thread
        void (*get_current_device)(TF_Platform* platform, int* out_device_index, TF_Status* out_status);
        void (*set_current_device)(TF_Platform* platform, int device_index, TF_Status* out_status);
        void (*get_device_for_pointer)(TF_Platform* platform, const void* pointer, int* out_device_index, TF_Status* out_status);
        void (*can_access_peer)(TF_Platform* platform, int device_index, int peer_device_index, bool* out_can_access, TF_Status* out_status);
        void (*get_native_handle)(TF_Platform* platform, void** out_handle);
    } TF_PlatformOps;
    #define TF_PLATFORM_STRUCT_SIZE TF_OFFSET_OF_END(TF_PlatformOps, get_native_handle)
    
    TF_CAPI_EXPORT void create_platform(TF_PlatformOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_platform(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_PLATFORM_H_
