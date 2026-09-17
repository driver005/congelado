#ifndef CONGELADO_C_EXTERN_JOB_TASK_H_
#define CONGELADO_C_EXTERN_JOB_TASK_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"
#include "c/extern/jobber/job.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Task
    {
        void* plugin_data;
    } TF_Task;

    typedef struct TF_TaskOps
    {
        size_t struct_size;

        void (*destroy)(TF_Task* task);

        void (*complete)(TF_Task* task, const TF_String* node_ref, const TF_String* output, TF_Status* out_status);

        void (*create_task)(TF_Task* task, const TF_String* node_ref, const TF_String* input, TF_Job* out_child, TF_Status* out_status);
        void (*get_task)(TF_Task* task, const TF_String* node_ref, TF_Job* out_child);
        void (*list_tasks)(TF_Task* task, TF_Vector* out_node_refs, TF_Status* out_status);
        void (*cancel_task)(TF_Task* task, const TF_String* node_ref, TF_Status* out_status);

    } TF_TaskOps;

#define TF_TASK_STRUCT_SIZE TF_OFFSET_OF_END(TF_TaskOps, cancel_task)

    TF_CAPI_EXPORT void create_task(
        TF_TaskOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_task(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_EXTERN_JOB_TASK_H_
