#ifndef TENSORFLOW_C_EXTERN_OPS_DIMENSION_HANDLE_H_
#define TENSORFLOW_C_EXTERN_OPS_DIMENSION_HANDLE_H_

#include "include/c/intern/status.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_DimensionHandle
    {
        void* plugin_data;
    } TF_DimensionHandle;

    // TF_DimensionHandleOps
    typedef struct TF_DimensionHandleOps
    {
        size_t struct_size;
        void (*create)(TF_DimensionHandle* out_handle);
        void (*destroy)(TF_DimensionHandle* handle);
        void (*value_known)(TF_DimensionHandle* dim_handle, int* out_known);
        void (*value)(TF_DimensionHandle* dim_handle, int64_t* out_value);
    } TF_DimensionHandleOps;

#define TF_DIMENSION_HANDLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DimensionHandleOps, value)
    TF_CAPI_EXPORT void create_dimension_handle(
        TF_DimensionHandleOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_dimension_handle(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_OPS_DIMENSION_HANDLE_H_
