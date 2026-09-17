#ifndef CONGELADO_C_EXTERN_JOBBER_H_
#define CONGELADO_C_EXTERN_JOBBER_H_

#include "include/c/macros.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"
#include "include/c/extern/jobber/job.h"
#include "include/c/extern/jobber/task.h"
#include "include/c/extern/jobber/schedule.h"
#include "include/c/extern/jobber/observe.h"
#include "include/c/extern/jobber/options.h"
#include "include/c/extern/jobber/durable.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Jobber
    {
        void* plugin_data;
        const TF_TaskOps* task_ops;
        const TF_ScheduleOps* schedule_ops;
        const TF_ObserveOps* observe_ops;
        const TF_OptionsOps* options_ops;
        const TF_DurableOps* durable_ops;
    } TF_Jobber;

    typedef struct TF_JobberOps
    {
        size_t struct_size;
        void (*destroy)(TF_Jobber* jobber);
        void (*get_name)(TF_Jobber* jobber, TF_String* out_name);
    } TF_JobberOps;

#define TF_JOBBER_STRUCT_SIZE TF_OFFSET_OF_END(TF_JobberOps, get_name)

    TF_CAPI_EXPORT void create_jobber(TF_JobberOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_jobber(void* plugin_context);

    static inline void init_jobber(TF_JobberOps** ops, TF_Jobber* jobber, TF_Status* out_status)
    {
        create_jobber(ops, &jobber->plugin_data, out_status);

        TF_TaskOps* task_ops = NULL;
        create_task(&task_ops, &jobber->plugin_data, out_status);
        jobber->task_ops = task_ops;

        TF_ScheduleOps* schedule_ops = NULL;
        create_schedule(&schedule_ops, &jobber->plugin_data, out_status);
        jobber->schedule_ops = schedule_ops;

        TF_ObserveOps* observe_ops = NULL;
        create_observe(&observe_ops, &jobber->plugin_data, out_status);
        jobber->observe_ops = observe_ops;

        TF_OptionsOps* options_ops = NULL;
        create_options(&options_ops, &jobber->plugin_data, out_status);
        jobber->options_ops = options_ops;

        TF_DurableOps* durable_ops = NULL;
        create_durable(&durable_ops, &jobber->plugin_data, out_status);
        jobber->durable_ops = durable_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_EXTERN_JOBBER_H_
