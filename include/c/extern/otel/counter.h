#ifndef CONGELADO_C_OTEL_COUNTER_H_
#define CONGELADO_C_OTEL_COUNTER_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Otel_Counter
    {
        void* plugin_data;
    } TF_Otel_Counter;

    typedef struct TF_Otel_CounterOps
    {
        size_t struct_size;
        void (*destroy)(TF_Otel_Counter* counter);
        void (*get_name)(TF_Otel_Counter* counter, TF_String* out_name);
        void (*add)(TF_Otel_Counter* counter, double value, TF_Status* out_status);
    } TF_Otel_CounterOps;

#define TF_OTEL_COUNTER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Otel_CounterOps, add)

    TF_CAPI_EXPORT void
    create_otel_counter(TF_Otel_CounterOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_counter(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_COUNTER_H_
