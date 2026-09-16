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

    // --------------------------------------------------------------------------
    // TF_TimePoint — plugin vtable for a point in time expressed as a duration since an implementation-defined epoch (std::chrono::time_point equivalent).
    //
    // TF_TimePoint is an opaque pointer to a plugin-owned time_point object.
    typedef struct TF_TimePoint
    {
        void* plugin_data;
    } TF_TimePoint;

    // Plugin-facing vtable registered via create_time_point.
    typedef struct TF_TimePointOps
    {
        size_t struct_size;

        // Allocate a new time_point ticks * (ratio_num / ratio_den) seconds since the epoch. Must be freed with destroy.
        TF_TimePoint* (*new_time_point)(
            void* plugin_context,
            int64_t ticks,
            int64_t ratio_num,
            int64_t ratio_den
        );

        // A duration handle (obtained via TF_Duration, see tf_duration.h) representing the elapsed time since the epoch. Owned by the time_point; do not destroy independently.
        TF_Duration* (*get_duration_since_epoch)(
            const TF_TimePoint* time_point
        );

        // Free a handle returned by new_time_point.
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
