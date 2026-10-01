#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_TIMER_H_
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
        void (*create)(TF_Timer* out_handle);
        void (*destroy)(TF_Timer* handle);
        void (*nanoseconds)(TF_Timer* timer, uint64_t* out_nanoseconds);
    } TF_TimerOps;
    #define TF_TIMER_STRUCT_SIZE TF_OFFSET_OF_END(TF_TimerOps, nanoseconds)
    
    TF_CAPI_EXPORT void create_timer(TF_TimerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_timer(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_TIMER_H_
