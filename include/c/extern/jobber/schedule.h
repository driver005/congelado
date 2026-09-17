#ifndef CONGELADO_C_EXTERN_JOB_SCHEDULE_H_
#define CONGELADO_C_EXTERN_JOB_SCHEDULE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_vector.h"
#include "c/extern/jobber/job.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Schedule
    {
        void* plugin_data;
    } TF_Schedule;

    typedef struct TF_ScheduleOps
    {
        size_t struct_size;

        void (*destroy)(TF_Schedule* schedule);

        void (*add_dependency)(TF_Schedule* schedule, TF_Job* job, TF_Job* depends_on, TF_Status* out_status);
        void (*list_dependencies)(TF_Schedule* schedule, TF_Job* job, TF_Vector* out_job_ids, TF_Status* out_status);

        void (*pause)(TF_Schedule* schedule, TF_Job* job, TF_Status* out_status);
        void (*resume)(TF_Schedule* schedule, TF_Job* job, TF_Status* out_status);
        void (*cancel)(TF_Schedule* schedule, TF_Job* job, TF_Status* out_status);
        void (*stop)(TF_Schedule* schedule, TF_Job* job, TF_Status* out_status);

    } TF_ScheduleOps;

#define TF_SCHEDULE_STRUCT_SIZE TF_OFFSET_OF_END(TF_ScheduleOps, stop)

    TF_CAPI_EXPORT void create_schedule(
        TF_ScheduleOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_schedule(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_EXTERN_JOB_SCHEDULE_H_
