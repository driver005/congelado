#ifndef TENSORFLOW_C_EXTERN_JOB_H_
#define TENSORFLOW_C_EXTERN_JOB_H_

#include "c/abi/macros.h"
#include "c/intern/tf_map.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/intern/tf_vector.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Job — generic async-work engine, replacing the separate orchestrator,
    // worker, cron, and manager domain vtables. All four are "submit a unit
    // of async work, track its completion/progress, control its lifecycle
    // (pause/stop/cancel), optionally on a schedule" — a workflow instance, a
    // task execution, a worker process, and a cron job are the same shape at
    // different granularities.
    //
    // Brought to the real depth production workflow/job engines need
    // (Temporal signals/checkpoints, Celery retry, Airflow DAG dependencies).
    typedef enum TF_Job_Status
    {
        TF_JOB_PENDING = 0,
        TF_JOB_RUNNING = 1,
        TF_JOB_PAUSED = 2,
        TF_JOB_COMPLETED = 3,
        TF_JOB_FAILED = 4,
        TF_JOB_CANCELLED = 5
    } TF_Job_Status;

    typedef struct TF_Job_Options
    {
        size_t struct_size;
        const TF_TString* cron_expression; // nullable; non-NULL => run on this schedule
        int priority;                      // higher runs first; 0 = default
        int max_retries;
        int64_t timeout_ms;                // 0 = no timeout
    } TF_Job_Options;

#define TF_JOB_OPTIONS_STRUCT_SIZE TF_OFFSET_OF_END(TF_Job_Options, timeout_ms)

    typedef struct TF_Job_Handle TF_Job_Handle;
    typedef void (*TF_Job_CompletionFn)(void* user_data, const TF_TString* output, TF_Status* status);
    typedef void (*TF_Job_ProgressFn)(void* user_data, const TF_TString* progress_payload);
    typedef void (*TF_Job_StatusFn)(void* user_data, TF_Job_Status job_status, TF_Status* status);
    typedef void (*TF_Job_ResultFn)(void* user_data, const TF_TString* output, TF_Status* status);
    typedef void (*TF_Job_AckFn)(void* user_data, TF_Status* status);

    // Plugin-facing vtable registered via init_job.
    typedef struct TF_Job
    {
        size_t struct_size;

        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        // Synchronous: blocks, returns output directly. Restores
        // worker.execute's original call shape; independent of
        // submit/on_complete below.
        void (*execute)(void* plugin_context, const TF_TString* input, TF_TString* out_output, TF_Status* status);

        // options may be NULL for "run once now, default priority, no
        // retry/timeout".
        TF_Job_Handle* (*submit)(void* plugin_context, const TF_TString* input, const TF_Job_Options* options, TF_Status* status);

        // Manual retry of a failed job.
        void (*resubmit)(void* plugin_context, TF_Job_Handle* job, TF_Status* status);

        // Block until the job reaches a terminal state; a synchronous
        // alternative to registering on_complete, for callers already on a
        // worker thread.
        void (*wait)(void* plugin_context, TF_Job_Handle* job, int64_t timeout_ms, TF_Job_ResultFn completion, void* user_data, TF_Status* status);

        void (*on_complete)(void* plugin_context, TF_Job_Handle* job, TF_Job_CompletionFn completion, void* user_data);
        void (*on_progress)(void* plugin_context, TF_Job_Handle* job, TF_Job_ProgressFn progress, void* user_data);

        // Poll current state/result/introspection without a pre-registered
        // callback.
        void (*get_status)(void* plugin_context, TF_Job_Handle* job, TF_Job_StatusFn completion, void* user_data, TF_Status* status);
        void (*get_result)(void* plugin_context, TF_Job_Handle* job, TF_Job_ResultFn completion, void* user_data, TF_Status* status);

        // Ordered status-change log.
        void (*get_history)(void* plugin_context, TF_Job_Handle* job, TF_Vector_Handle* out_transitions, TF_Status* status);

        // out_metrics keys such as duration_ms/retry_count are a documented
        // convention, not enforced by this header.
        void (*get_metrics)(void* plugin_context, TF_Job_Handle* job, TF_Map_Handle* out_metrics, TF_Status* status);
        void (*get_logs)(void* plugin_context, TF_Job_Handle* job, TF_Vector_Handle* out_lines, TF_Status* status);

        // Introspect/modify an already-submitted job's scheduling options.
        void (*get_options)(void* plugin_context, TF_Job_Handle* job, TF_Job_Options* out_options, TF_Status* status);
        void (*update_options)(void* plugin_context, TF_Job_Handle* job, const TF_Job_Options* options, TF_Status* status);
        void (*set_priority)(void* plugin_context, TF_Job_Handle* job, int priority, TF_Status* status);

        // DAG dependencies — job B waits for job A, matching real workflow
        // engines.
        void (*add_dependency)(void* plugin_context, TF_Job_Handle* job, TF_Job_Handle* depends_on, TF_Status* status);
        void (*list_dependencies)(void* plugin_context, TF_Job_Handle* job, TF_Vector_Handle* out_job_ids, TF_Status* status);

        void (*pause)(void* plugin_context, TF_Job_Handle* job, TF_Status* status);
        void (*resume)(void* plugin_context, TF_Job_Handle* job, TF_Status* status);
        void (*cancel)(void* plugin_context, TF_Job_Handle* job, TF_Status* status);
        void (*stop)(void* plugin_context, TF_Job_Handle* job, TF_Status* status);

        // Send an arbitrary external event into a running job (Temporal-style
        // signal), and durable-execution checkpoint/restore for long-running
        // jobs.
        void (*signal)(void* plugin_context, TF_Job_Handle* job, const TF_TString* signal_name, const TF_TString* payload, TF_Status* status);
        void (*checkpoint)(void* plugin_context, TF_Job_Handle* job, TF_Job_AckFn completion, void* user_data, TF_Status* status);
        void (*restore_checkpoint)(void* plugin_context, TF_Job_Handle* job, TF_Status* status);

        // node_ref: NULL completes the whole job; non-NULL completes one
        // named task within a multi-task workflow instance
        // (orchestrator.complete_task).
        void (*complete)(void* plugin_context, TF_Job_Handle* job, const TF_TString* node_ref, const TF_TString* output, TF_Status* status);

        // Task sub-tree — a workflow job can have named child tasks,
        // mirroring generator's Function->Parameter parent->child
        // introspection.
        TF_Job_Handle* (*create_task)(void* plugin_context, TF_Job_Handle* parent, const TF_TString* node_ref, const TF_TString* input, TF_Status* status);
        TF_Job_Handle* (*get_task)(void* plugin_context, TF_Job_Handle* parent, const TF_TString* node_ref, TF_Status* status);
        void (*list_tasks)(void* plugin_context, TF_Job_Handle* parent, TF_Vector_Handle* out_node_refs, TF_Status* status);
        void (*cancel_task)(void* plugin_context, TF_Job_Handle* parent, const TF_TString* node_ref, TF_Status* status);

        void (*list)(void* plugin_context, const TF_Map_Handle* filters, TF_Vector_Handle* out_job_ids, TF_Status* status);
        void (*destroy_job)(void* plugin_context, TF_Job_Handle* job);

    } TF_Job;

#define TF_JOB_STRUCT_SIZE TF_OFFSET_OF_END(TF_Job, destroy_job)

    TF_CAPI_EXPORT void init_job(TF_Job** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_JOB_H_
