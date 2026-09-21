import os

files = {
    'include/c/extern/stream_executor/stream.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Stream { void* plugin_data; } TF_Stream;

    // TF_StreamOps
    typedef struct TF_StreamOps {
        size_t struct_size;
    } TF_StreamOps;
    #define TF_STREAM_STRUCT_SIZE TF_OFFSET_OF_END(TF_StreamOps, struct_size)
    
    TF_CAPI_EXPORT void create_stream(TF_StreamOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_stream(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_
''',

    'include/c/extern/stream_executor/event.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Event { void* plugin_data; } TF_Event;

    typedef enum TF_EventStatus {
        TF_EVENT_UNKNOWN,
        TF_EVENT_ERROR,
        TF_EVENT_PENDING,
        TF_EVENT_COMPLETE,
    } TF_EventStatus;

    // TF_EventOps
    typedef struct TF_EventOps {
        size_t struct_size;
    } TF_EventOps;
    #define TF_EVENT_STRUCT_SIZE TF_OFFSET_OF_END(TF_EventOps, struct_size)
    
    TF_CAPI_EXPORT void create_event(TF_EventOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_event(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_
''',

    'include/c/extern/stream_executor/timer.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_TIMER_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_TIMER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Timer { void* plugin_data; } TF_Timer;

    // TF_TimerOps
    typedef struct TF_TimerOps {
        size_t struct_size;
        void (*nanoseconds)(TF_Timer* timer, uint64_t* out_nanoseconds);
    } TF_TimerOps;
    #define TF_TIMER_STRUCT_SIZE TF_OFFSET_OF_END(TF_TimerOps, nanoseconds)
    
    TF_CAPI_EXPORT void create_timer(TF_TimerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_timer(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_TIMER_H_
''',

    'include/c/extern/stream_executor/device.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Device { void* plugin_data; } TF_Device;

    // TF_DeviceOps
    typedef struct TF_DeviceOps {
        size_t struct_size;
        void (*get_numa_node)(TF_Device* device, int32_t* out_numa_node);
        void (*get_memory_bandwidth)(TF_Device* device, int64_t* out_bandwidth);
        void (*get_gflops)(TF_Device* device, double* out_gflops);
    } TF_DeviceOps;
    #define TF_DEVICE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DeviceOps, get_gflops)
    
    TF_CAPI_EXPORT void create_device(TF_DeviceOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_device(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_
''',

    'include/c/extern/stream_executor/executor.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EXECUTOR_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EXECUTOR_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/extern/stream_executor/device.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_StreamExecutor { void* plugin_data; } TF_StreamExecutor;

    typedef struct TF_DeviceMemoryBase {
        size_t struct_size;
        void* ext;
        void* opaque;
        uint64_t size;
        uint64_t payload;
    } TF_DeviceMemoryBase;

    typedef struct TF_AllocatorStats {
        size_t struct_size;
        int64_t num_allocs;
        int64_t bytes_in_use;
        int64_t peak_bytes_in_use;
        int64_t largest_alloc_size;
        int8_t has_bytes_limit;
        int64_t bytes_limit;
        int64_t bytes_reserved;
        int64_t peak_bytes_reserved;
        int8_t has_bytes_reservable_limit;
        int64_t bytes_reservable_limit;
        int64_t largest_free_block_bytes;
    } TF_AllocatorStats;

    // TF_StreamExecutorOps
    typedef struct TF_StreamExecutorOps {
        size_t struct_size;
        void (*allocate)(TF_StreamExecutor* executor, TF_Device* device, uint64_t size, int64_t memory_space, TF_DeviceMemoryBase* mem);
        void (*deallocate)(TF_StreamExecutor* executor, TF_Device* device, TF_DeviceMemoryBase* memory);
        void (*host_memory_allocate)(TF_StreamExecutor* executor, TF_Device* device, uint64_t size, void** out_mem);
        void (*host_memory_deallocate)(TF_StreamExecutor* executor, TF_Device* device, void* mem);
        void (*unified_memory_allocate)(TF_StreamExecutor* executor, TF_Device* device, uint64_t bytes, void** out_location);
        void (*unified_memory_deallocate)(TF_StreamExecutor* executor, TF_Device* device, void* location);
        void (*get_allocator_stats)(TF_StreamExecutor* executor, TF_Device* device, TF_AllocatorStats* stats, bool* out_success);
        void (*device_memory_usage)(TF_StreamExecutor* executor, TF_Device* device, int64_t* out_free, int64_t* out_total, bool* out_success);
        void (*create_stream_internal)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*destroy_stream_internal)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream);
        void (*create_stream_dependency)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* dependent, TF_Stream* other, TF_Status* out_status);
        void (*get_stream_status)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*create_event_internal)(TF_StreamExecutor* executor, TF_Device* device, TF_Event* event, TF_Status* out_status);
        void (*destroy_event_internal)(TF_StreamExecutor* executor, TF_Device* device, TF_Event* event);
        void (*get_event_status)(TF_StreamExecutor* executor, TF_Device* device, TF_Event* event, TF_EventStatus* out_event_status);
        void (*record_event)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Event* event, TF_Status* out_status);
        void (*wait_for_event)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Event* event, TF_Status* out_status);
        void (*create_timer_internal)(TF_StreamExecutor* executor, TF_Device* device, TF_Timer* timer, TF_Status* out_status);
        void (*destroy_timer_internal)(TF_StreamExecutor* executor, TF_Device* device, TF_Timer* timer);
        void (*start_timer)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Timer* timer, TF_Status* out_status);
        void (*stop_timer)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Timer* timer, TF_Status* out_status);
        void (*memcpy_dtoh)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, void* host_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*memcpy_htod)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* device_dst, const void* host_src, uint64_t size, TF_Status* out_status);
        void (*memcpy_dtod)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* device_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*sync_memcpy_dtoh)(TF_StreamExecutor* executor, TF_Device* device, void* host_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*sync_memcpy_htod)(TF_StreamExecutor* executor, TF_Device* device, TF_DeviceMemoryBase* device_dst, const void* host_src, uint64_t size, TF_Status* out_status);
        void (*sync_memcpy_dtod)(TF_StreamExecutor* executor, TF_Device* device, TF_DeviceMemoryBase* device_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*block_host_for_event)(TF_StreamExecutor* executor, TF_Device* device, TF_Event* event, TF_Status* out_status);
        void (*block_host_until_done)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*synchronize_all_activity)(TF_StreamExecutor* executor, TF_Device* device, TF_Status* out_status);
        void (*mem_zero)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* location, uint64_t size, TF_Status* out_status);
        void (*memset)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* location, uint8_t pattern, uint64_t size, TF_Status* out_status);
        void (*memset32)(TF_StreamExecutor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* location, uint32_t pattern, uint64_t size, TF_Status* out_status);
    } TF_StreamExecutorOps;
    #define TF_STREAM_EXECUTOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_StreamExecutorOps, memset32)
    
    TF_CAPI_EXPORT void create_stream_executor(TF_StreamExecutorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_stream_executor(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EXECUTOR_H_
''',

    'include/c/extern/stream_executor/platform.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_PLATFORM_H_
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
        void (*create_stream_executor_internal)(TF_Platform* platform, TF_StreamExecutor* executor, TF_Status* out_status);
        void (*destroy_stream_executor_internal)(TF_Platform* platform, TF_StreamExecutor* executor);
    } TF_PlatformOps;
    #define TF_PLATFORM_STRUCT_SIZE TF_OFFSET_OF_END(TF_PlatformOps, destroy_stream_executor_internal)
    
    TF_CAPI_EXPORT void create_platform(TF_PlatformOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_platform(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_PLATFORM_H_
''',

    'include/c/extern/stream_executor/stream_executor.h': '''#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_EXECUTOR_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_EXECUTOR_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/executor.h"
#include "include/c/extern/stream_executor/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_StreamExecutorFacade {
        void* plugin_data;
        const TF_StreamOps* stream_ops;
        const TF_EventOps* event_ops;
        const TF_TimerOps* timer_ops;
        const TF_DeviceOps* device_ops;
        const TF_StreamExecutorOps* executor_ops;
        const TF_PlatformOps* platform_ops;
    } TF_StreamExecutorFacade;

    typedef struct TF_StreamExecutorFacadeOps {
        size_t struct_size;
        void (*destroy)(TF_StreamExecutorFacade* facade);
        void (*get_name)(TF_StreamExecutorFacade* facade, TF_String* out_name);
    } TF_StreamExecutorFacadeOps;

    #define TF_STREAM_EXECUTOR_FACADE_STRUCT_SIZE TF_OFFSET_OF_END(TF_StreamExecutorFacadeOps, get_name)

    TF_CAPI_EXPORT void create_stream_executor_facade(TF_StreamExecutorFacadeOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_stream_executor_facade(void* plugin_context);

    static inline void init_stream_executor_facade(TF_StreamExecutorFacadeOps** ops, TF_StreamExecutorFacade* facade, TF_Status* out_status) {
        create_stream_executor_facade(ops, &facade->plugin_data, out_status);

        TF_StreamOps* stream_ops = NULL;
        create_stream(&stream_ops, &facade->plugin_data, out_status);
        facade->stream_ops = stream_ops;

        TF_EventOps* event_ops = NULL;
        create_event(&event_ops, &facade->plugin_data, out_status);
        facade->event_ops = event_ops;

        TF_TimerOps* timer_ops = NULL;
        create_timer(&timer_ops, &facade->plugin_data, out_status);
        facade->timer_ops = timer_ops;

        TF_DeviceOps* device_ops = NULL;
        create_device(&device_ops, &facade->plugin_data, out_status);
        facade->device_ops = device_ops;

        TF_StreamExecutorOps* executor_ops = NULL;
        create_stream_executor(&executor_ops, &facade->plugin_data, out_status);
        facade->executor_ops = executor_ops;

        TF_PlatformOps* platform_ops = NULL;
        create_platform(&platform_ops, &facade->plugin_data, out_status);
        facade->platform_ops = platform_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_EXECUTOR_H_
'''
}

for filepath, content in files.items():
    with open(filepath, 'w') as f:
        f.write(content)
