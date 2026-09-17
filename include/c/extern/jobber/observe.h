#ifndef CONGELADO_C_EXTERN_JOB_OBSERVE_H_
#define CONGELADO_C_EXTERN_JOB_OBSERVE_H_

#include "c/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_vector.h"
#include "c/extern/jobber/job.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TF_Observe_StatusFn)(void* user_data, TF_Job_Status job_status, TF_Status* out_status);
    typedef void (*TF_Observe_ResultFn)(void* user_data, const TF_String* output, TF_Status* out_status);

    typedef struct TF_Observe
    {
        void* plugin_data;
    } TF_Observe;

    typedef struct TF_ObserveOps
    {
        size_t struct_size;

        void (*destroy)(TF_Observe* observe);

        void (*get_status)(TF_Observe* observe, TF_Job* job, TF_Observe_StatusFn completion, void* user_data, TF_Status* out_status);
        void (*get_result)(TF_Observe* observe, TF_Job* job, TF_Observe_ResultFn completion, void* user_data, TF_Status* out_status);

        void (*get_history)(TF_Observe* observe, TF_Job* job, TF_Vector* out_transitions, TF_Status* out_status);
        void (*get_metrics)(TF_Observe* observe, TF_Job* job, TF_Map* out_metrics, TF_Status* out_status);
        void (*get_logs)(TF_Observe* observe, TF_Job* job, TF_Vector* out_lines, TF_Status* out_status);

    } TF_ObserveOps;

#define TF_OBSERVE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ObserveOps, get_logs)

    TF_CAPI_EXPORT void create_observe(
        TF_ObserveOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_observe(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_EXTERN_JOB_OBSERVE_H_
