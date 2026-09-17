#ifndef CONGELADO_C_EXTERN_JOB_DURABLE_H_
#define CONGELADO_C_EXTERN_JOB_DURABLE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/jobber/job.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TFDurableAckFn)(void* user_data, TF_Status* out_status);

    typedef struct TF_Durable
    {
        void* plugin_data;
    } TF_Durable;

    typedef struct TF_DurableOps
    {
        size_t struct_size;

        void (*destroy)(TF_Durable* durable);

        void (*signal)(TF_Durable* durable, TF_Job* job, const TF_String* signal_name, const TF_String* payload, TF_Status* out_status);
        void (*checkpoint)(TF_Durable* durable, TF_Job* job, TFDurableAckFn completion, void* user_data, TF_Status* out_status);
        void (*restore_checkpoint)(TF_Durable* durable, TF_Job* job, TF_Status* out_status);

    } TF_DurableOps;

#define TF_DURABLE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DurableOps, restore_checkpoint)

    TF_CAPI_EXPORT void create_durable(
        TF_DurableOps** ops,
        void** plugin_context,
        TF_Status* out_status
    );
    TF_CAPI_EXPORT void destroy_durable(void* plugin_context);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_EXTERN_JOB_DURABLE_H_
