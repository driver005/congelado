#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_EXECUTOR_H_
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

    typedef struct TF_StreamExecutor {
        void* plugin_data;
        void* stream_context;
        void* event_context;
        void* timer_context;
        void* device_context;
        void* executor_context;
        void* platform_context;
        const TF_StreamOps* stream_ops;
        const TF_EventOps* event_ops;
        const TF_TimerOps* timer_ops;
        const TF_DeviceOps* device_ops;
        const TF_ExecutorOps* executor_ops;
        const TF_PlatformOps* platform_ops;
    } TF_StreamExecutor;

    typedef struct TF_StreamExecutorOps {
        size_t struct_size;
        void (*destroy)(TF_StreamExecutor* facade);
        void (*get_name)(TF_StreamExecutor* facade, TF_String* out_name);
    } TF_StreamExecutorOps;

    #define TF_STREAM_EXECUTOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_StreamExecutorOps, get_name)

    TF_CAPI_EXPORT void create_stream_executor(TF_StreamExecutorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_stream_executor(void* plugin_context);

    // Each vtable gets its own plugin context slot so create_* calls do not overwrite one another.
    static inline void init_stream_executor(TF_StreamExecutorOps** ops, TF_StreamExecutor* facade, TF_Status* out_status) {
        create_stream_executor(ops, &facade->plugin_data, out_status);

        TF_StreamOps* stream_ops = NULL;
        create_stream(&stream_ops, &facade->stream_context, out_status);
        facade->stream_ops = stream_ops;

        TF_EventOps* event_ops = NULL;
        create_event(&event_ops, &facade->event_context, out_status);
        facade->event_ops = event_ops;

        TF_TimerOps* timer_ops = NULL;
        create_timer(&timer_ops, &facade->timer_context, out_status);
        facade->timer_ops = timer_ops;

        TF_DeviceOps* device_ops = NULL;
        create_device(&device_ops, &facade->device_context, out_status);
        facade->device_ops = device_ops;

        TF_ExecutorOps* executor_ops = NULL;
        create_executor(&executor_ops, &facade->executor_context, out_status);
        facade->executor_ops = executor_ops;

        TF_PlatformOps* platform_ops = NULL;
        create_platform(&platform_ops, &facade->platform_context, out_status);
        facade->platform_ops = platform_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_EXECUTOR_H_
