#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EXECUTOR_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EXECUTOR_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/event.h"
#include "include/c/extern/stream_executor/timer.h"
#include "include/c/extern/stream_executor/device.h"
#include "include/c/extern/stream_executor/types.h"
#include "include/c/extern/stream_executor/allocator.h"
#include "include/c/extern/random_generator/random_generator.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Executor { void* plugin_data; } TF_Executor;

    typedef void (*TF_StatusCallbackFn)(void* user_data, TF_Status* out_status);

    typedef struct TF_StreamOptions {
        size_t struct_size;
        int32_t priority;
    } TF_StreamOptions;
    #define TF_STREAM_OPTIONS_STRUCT_SIZE TF_OFFSET_OF_END(TF_StreamOptions, priority)

    // TF_ExecutorOps
    typedef struct TF_ExecutorOps {
        size_t struct_size;
        void (*create)(TF_Executor* out_handle);
        void (*destroy)(TF_Executor* handle);
        void (*device_memory_usage)(TF_Executor* executor, TF_Device* device, int64_t* out_free, int64_t* out_total, bool* out_success);
        void (*create_stream_internal)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*destroy_stream_internal)(TF_Executor* executor, TF_Device* device, TF_Stream* stream);
        void (*create_stream_dependency)(TF_Executor* executor, TF_Device* device, TF_Stream* dependent, TF_Stream* other, TF_Status* out_status);
        void (*get_stream_status)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*create_event_internal)(TF_Executor* executor, TF_Device* device, TF_Event* event, TF_Status* out_status);
        void (*destroy_event_internal)(TF_Executor* executor, TF_Device* device, TF_Event* event);
        void (*get_event_status)(TF_Executor* executor, TF_Device* device, TF_Event* event, TF_EventStatus* out_event_status);
        void (*record_event)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Event* event, TF_Status* out_status);
        void (*wait_for_event)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Event* event, TF_Status* out_status);
        void (*create_timer_internal)(TF_Executor* executor, TF_Device* device, TF_Timer* timer, TF_Status* out_status);
        void (*destroy_timer_internal)(TF_Executor* executor, TF_Device* device, TF_Timer* timer);
        void (*start_timer)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Timer* timer, TF_Status* out_status);
        void (*stop_timer)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Timer* timer, TF_Status* out_status);
        void (*memcpy_dtoh)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, void* host_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*memcpy_htod)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* device_dst, const void* host_src, uint64_t size, TF_Status* out_status);
        void (*memcpy_dtod)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* device_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*sync_memcpy_dtoh)(TF_Executor* executor, TF_Device* device, void* host_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*sync_memcpy_htod)(TF_Executor* executor, TF_Device* device, TF_DeviceMemoryBase* device_dst, const void* host_src, uint64_t size, TF_Status* out_status);
        void (*sync_memcpy_dtod)(TF_Executor* executor, TF_Device* device, TF_DeviceMemoryBase* device_dst, const TF_DeviceMemoryBase* device_src, uint64_t size, TF_Status* out_status);
        void (*block_host_for_event)(TF_Executor* executor, TF_Device* device, TF_Event* event, TF_Status* out_status);
        void (*block_host_until_done)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*synchronize_all_activity)(TF_Executor* executor, TF_Device* device, TF_Status* out_status);
        void (*mem_zero)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* location, uint64_t size, TF_Status* out_status);
        void (*memset)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* location, uint8_t pattern, uint64_t size, TF_Status* out_status);
        void (*memset32)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_DeviceMemoryBase* location, uint32_t pattern, uint64_t size, TF_Status* out_status);
        void (*host_callback)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_StatusCallbackFn callback_fn, void* callback_arg, bool* out_success);
        void (*create_stream_with_options)(TF_Executor* executor, TF_Device* device, const TF_StreamOptions* options, TF_Stream* stream, TF_Status* out_status);
        // stream pool + current stream (pool streams are borrowed, never pass them to destroy_stream_internal). Current stream is per calling thread.
        void (*get_stream_from_pool)(TF_Executor* executor, TF_Device* device, int32_t priority, TF_Stream* out_stream, TF_Status* out_status);
        void (*get_current_stream)(TF_Executor* executor, TF_Device* device, TF_Stream* out_stream, TF_Status* out_status);
        void (*set_current_stream)(TF_Executor* executor, TF_Device* device, TF_Stream* stream, TF_Status* out_status);
        void (*create_stream_from_native)(TF_Executor* executor, TF_Device* device, void* native_handle, TF_Stream* out_stream, TF_Status* out_status);
        // events
        void (*create_event_with_options_internal)(TF_Executor* executor, TF_Device* device, const TF_EventOptions* options, TF_Event* out_event, TF_Status* out_status);
        void (*create_event_from_ipc_internal)(TF_Executor* executor, TF_Device* device, const TF_IpcEventHandle* handle, TF_Event* out_event, TF_Status* out_status);
        // child handles
        void (*create_allocator_internal)(TF_Executor* executor, TF_Device* device, TF_Allocator* out_allocator, TF_Status* out_status);
        void (*destroy_allocator_internal)(TF_Executor* executor, TF_Device* device, TF_Allocator* allocator);
        void (*create_random_generator_internal)(TF_Executor* executor, TF_Device* device, uint64_t seed, TF_RandomGenerator* out_generator, TF_Status* out_status);
        void (*destroy_random_generator_internal)(TF_Executor* executor, TF_Device* device, TF_RandomGenerator* generator);
        void (*get_default_random_generator)(TF_Executor* executor, TF_Device* device, TF_RandomGenerator* out_generator, TF_Status* out_status);
        void (*get_native_handle)(TF_Executor* executor, TF_Device* device, void** out_handle);
    } TF_ExecutorOps;
    #define TF_EXECUTOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_ExecutorOps, get_native_handle)
    
    TF_CAPI_EXPORT void create_executor(TF_ExecutorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_executor(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EXECUTOR_H_
