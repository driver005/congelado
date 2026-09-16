#ifndef TENSORFLOW_C_TF_TIME_POINT_H_
#define TENSORFLOW_C_TF_TIME_POINT_H_

#include "c/macros.h"
#include "c/intern/tf_duration.h"
#include "c/intern/tf_status.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_TimePoint
    {
        void* plugin_data;
    } TF_TimePoint;

    typedef struct TF_TimePointOps
    {
        size_t struct_size;

        TF_Duration* (*get_duration_since_epoch)(const TF_TimePoint* time_point);
        void (*destroy)(TF_TimePoint* time_point);

    } TF_TimePointOps;

#define TF_TIME_POINT_STRUCT_SIZE TF_OFFSET_OF_END(TF_TimePointOps, destroy)

    TF_CAPI_EXPORT void
    create_time_point(TF_TimePointOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_time_point(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TIME_POINT_H_
