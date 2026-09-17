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

    typedef struct TF_Duration
    {
        void* plugin_data;
    } TF_Duration;

    typedef struct TF_DurationOps
    {
        size_t struct_size;

        void (*get_ticks)(const TF_Duration* duration, int64_t* out_ticks);
        void (*get_ratio_num)(const TF_Duration* duration, int64_t* out_num);
        void (*get_ratio_den)(const TF_Duration* duration, int64_t* out_den);
        void (*destroy)(TF_Duration* duration);

    } TF_DurationOps;

#define TF_DURATION_STRUCT_SIZE TF_OFFSET_OF_END(TF_DurationOps, destroy)

    TF_CAPI_EXPORT void create_duration(TF_DurationOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_duration(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_DURATION_H_
