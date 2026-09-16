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
    // TF_Duration — plugin vtable for a tick count against a rational period
    // (std::chrono::duration equivalent): ticks * (ratio_num / ratio_den)
    // seconds.
    //
    // TF_Duration_Handle is an opaque pointer to a plugin-owned duration
    // object.
    typedef struct TF_Duration_Handle TF_Duration_Handle;

    // Plugin-facing vtable registered via init_duration.
    typedef struct TF_Duration
    {
        size_t struct_size;

        // Allocate a new duration of ticks * (ratio_num / ratio_den)
        // seconds. Must be freed with destroy.
        TF_Duration_Handle* (*new_duration)(
            void* plugin_context,
            int64_t ticks,
            int64_t ratio_num,
            int64_t ratio_den
        );

        // The raw tick count, as given to new_duration.
        int64_t (*get_ticks)(const TF_Duration_Handle* duration);

        // The period's numerator, as given to new_duration.
        int64_t (*get_ratio_num)(const TF_Duration_Handle* duration);

        // The period's denominator, as given to new_duration.
        int64_t (*get_ratio_den)(const TF_Duration_Handle* duration);

        // Free a handle returned by new_duration.
        void (*destroy)(void* plugin_context, TF_Duration_Handle* duration);

    } TF_Duration;

#define TF_DURATION_STRUCT_SIZE TF_OFFSET_OF_END(TF_Duration, destroy)

    TF_CAPI_EXPORT void init_duration(TF_Duration** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_DURATION_H_
