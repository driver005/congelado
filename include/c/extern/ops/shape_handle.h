#ifndef TENSORFLOW_C_EXTERN_OPS_SHAPE_HANDLE_H_
#define TENSORFLOW_C_EXTERN_OPS_SHAPE_HANDLE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_ShapeHandle { void* plugin_data; } TF_ShapeHandle;

    // TF_ShapeHandleOps
    typedef struct TF_ShapeHandleOps {
        size_t struct_size;
    } TF_ShapeHandleOps;
    #define TF_SHAPE_HANDLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ShapeHandleOps, struct_size)
    TF_CAPI_EXPORT void create_shape_handle(TF_ShapeHandleOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_shape_handle(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_OPS_SHAPE_HANDLE_H_
