#ifndef CONGELADO_C_EXTERN_JOB_OPTIONS_H_
#define CONGELADO_C_EXTERN_JOB_OPTIONS_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/jobber/job.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Job_Options
    {
        size_t struct_size;
        const TF_String* cron_expression;
        int priority;
        int max_retries;
        int64_t timeout_ms;
    } TF_Job_Options;

#define TF_JOB_OPTIONS_STRUCT_SIZE TF_OFFSET_OF_END(TF_Job_Options, timeout_ms)

    typedef struct TF_Options
    {
        void* plugin_data;
    } TF_Options;

    typedef struct TF_OptionsOps
    {
        size_t struct_size;

        void (*destroy)(TF_Options* options);

        void (*get_options)(TF_Options* options, TF_Job* job, TF_Job_Options* out_options, TF_Status* out_status);
        void (*update_options)(TF_Options* options, TF_Job* job, const TF_Job_Options* new_options, TF_Status* out_status);
        void (*set_priority)(TF_Options* options, TF_Job* job, int priority, TF_Status* out_status);

    } TF_OptionsOps;

#define TF_OPTIONS_STRUCT_SIZE TF_OFFSET_OF_END(TF_OptionsOps, set_priority)

    TF_CAPI_EXPORT void create_options(
        TF_OptionsOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_options(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_EXTERN_JOB_OPTIONS_H_
