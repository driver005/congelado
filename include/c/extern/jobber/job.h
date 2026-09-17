#ifndef CONGELADO_C_EXTERN_JOB_H_
#define CONGELADO_C_EXTERN_JOB_H_

#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"
#include "include/c/macros.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


    typedef enum TFJobStatus
    {
        TF_JOB_PENDING = 0,
        TF_JOB_RUNNING = 1,
        TF_JOB_PAUSED = 2,
        TF_JOB_COMPLETED = 3,
        TF_JOB_FAILED = 4,
        TF_JOB_CANCELLED = 5
    } TFJobStatus;

    typedef void (*TFJobCompletionFn)(
        void* user_data,
        const TF_String* output,
        TF_Status* out_status
    );
    typedef void (*TFJobProgressFn)(void* user_data, const TF_String* progress_payload);

    typedef struct TF_Job
    {
        void* plugin_data;
    } TF_Job;

    typedef struct TF_JobOps
    {
        size_t struct_size;

        void (*destroy)(TF_Job* job);
        void (*get_name)(TF_Job* job, TF_String* out_name);

        void (*execute)(
            TF_Job* job,
            const TF_String* input,
            TF_String* out_output,
            TF_Status* out_status
        );
        void (*resubmit)(TF_Job* job, TF_Status* out_status);
        void (*wait)(
            TF_Job* job,
            int64_t timeout_ms,
            TFJobCompletionFn completion,
            void* user_data,
            TF_Status* out_status
        );

        void (*on_complete)(TF_Job* job, TFJobCompletionFn completion, void* user_data);
        void (*on_progress)(TF_Job* job, TFJobProgressFn progress, void* user_data);

        void (*list)(
            TF_Job* job,
            const TF_Map* filters,
            TF_Vector* out_job_ids,
            TF_Status* out_status
        );

    } TF_JobOps;

#define TF_JOB_STRUCT_SIZE TF_OFFSET_OF_END(TF_JobOps, list)

    TF_CAPI_EXPORT void create_job(TF_JobOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_job(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_EXTERN_JOB_H_
