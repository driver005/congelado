#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_

#include "include/c/intern/status.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Event
    {
        void* plugin_data;
    } TF_Event;

    typedef enum TF_EventStatus
    {
        TF_EVENT_UNKNOWN,
        TF_EVENT_ERROR,
        TF_EVENT_PENDING,
        TF_EVENT_COMPLETE,
    } TF_EventStatus;

    typedef struct TF_EventOptions
    {
        size_t struct_size;
        bool enable_timing;
        bool enable_ipc;
        bool reusable;
    } TF_EventOptions;

    // Opaque cross-process handle for an event. data_size bytes of data are valid.
    typedef struct TF_IpcEventHandle
    {
        size_t struct_size;
        uint8_t data[64];
        uint64_t data_size;
    } TF_IpcEventHandle;

    // TF_EventOps
    typedef struct TF_EventOps
    {
        size_t struct_size;
        void (*create)(TF_Event* out_handle);
        void (*destroy)(TF_Event* handle);
        void (*elapsed_time)(
            TF_Event* start,
            TF_Event* end,
            float* out_milliseconds,
            TF_Status* out_status
        );
        void (*export_ipc)(TF_Event* event, TF_IpcEventHandle* out_handle, TF_Status* out_status);
        void (*get_native_handle)(TF_Event* event, void** out_handle);
    } TF_EventOps;

#define TF_EVENT_STRUCT_SIZE TF_OFFSET_OF_END(TF_EventOps, get_native_handle)

    TF_CAPI_EXPORT void
    create_event(TF_EventOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_event(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_EVENT_H_
