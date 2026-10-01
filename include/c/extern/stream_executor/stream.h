#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_

#include "include/c/intern/status.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Stream
    {
        void* plugin_data;
    } TF_Stream;

    typedef enum TF_CaptureStatus
    {
        TF_CAPTURE_STATUS_NONE = 0,
        TF_CAPTURE_STATUS_ACTIVE = 1,
    } TF_CaptureStatus;

    // TF_StreamOps
    typedef struct TF_StreamOps
    {
        size_t struct_size;
        void (*create)(TF_Stream* out_handle);
        void (*destroy)(TF_Stream* handle);
        void (*get_priority)(TF_Stream* stream, int32_t* out_priority);
        void (*get_device_index)(TF_Stream* stream, int* out_device_index);
        void (*query)(TF_Stream* stream, bool* out_idle, TF_Status* out_status);
        void (*synchronize)(TF_Stream* stream, TF_Status* out_status);
        void (*get_capture_status)(
            TF_Stream* stream,
            TF_CaptureStatus* out_capture_status,
            TF_Status* out_status
        );
        void (*get_native_handle)(TF_Stream* stream, void** out_handle);
    } TF_StreamOps;

#define TF_STREAM_STRUCT_SIZE TF_OFFSET_OF_END(TF_StreamOps, get_native_handle)

    TF_CAPI_EXPORT void
    create_stream(TF_StreamOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_stream(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_
