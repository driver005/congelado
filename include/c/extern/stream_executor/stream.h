#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_STREAM_H_
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
