#ifndef TENSORFLOW_C_TF_DURATION_H_
#define TENSORFLOW_C_TF_DURATION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Duration — plugin vtable for a tick count against a rational period (std::chrono::duration equivalent): ticks * (ratio_num / ratio_den) seconds.
    //
    // TF_Duration is an opaque pointer to a plugin-owned duration object.
    typedef struct TF_Duration
    {
        void* plugin_data;
    } TF_Duration;

    // Plugin-facing vtable registered via create_duration.
    typedef struct TF_DurationOps
    {
        size_t struct_size;

        // Allocate a new duration of ticks * (ratio_num / ratio_den) seconds. Must be freed with destroy.
        TF_Duration* (*new_duration)(
            void* plugin_context,
            int64_t ticks,
            int64_t ratio_num,
            int64_t ratio_den
        );

        // The raw tick count, as given to new_duration.
        int64_t (*get_ticks)(const TF_Duration* duration);

        // The period's numerator, as given to new_duration.
        int64_t (*get_ratio_num)(const TF_Duration* duration);

        // The period's denominator, as given to new_duration.
        int64_t (*get_ratio_den)(const TF_Duration* duration);

        // Free a handle returned by new_duration.
        void (*destroy)(TF_Duration* duration);

    } TF_DurationOps;

#define TF_DURATION_STRUCT_SIZE TF_OFFSET_OF_END(TF_DurationOps, destroy)

    TF_CAPI_EXPORT void create_duration(TF_DurationOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_duration(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_DURATION_H_
