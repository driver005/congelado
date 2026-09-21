#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_
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
