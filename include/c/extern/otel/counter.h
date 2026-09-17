#ifndef CONGELADO_C_OTEL_COUNTER_H_
#define CONGELADO_C_OTEL_COUNTER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFOtelCounter
    {
        void* plugin_data;
    } TFOtelCounter;

    typedef struct TFOtelCounterOps
    {
        size_t struct_size;
        void (*destroy)(TFOtelCounter* counter);
        void (*get_name)(TFOtelCounter* counter, TF_String* out_name);
        void (*add)(TFOtelCounter* counter, double value, TF_Status* out_status);
    } TFOtelCounterOps;

#define TF_OTEL_COUNTER_STRUCT_SIZE TF_OFFSET_OF_END(TFOtelCounterOps, add)

    TF_CAPI_EXPORT void
    create_otel_counter(TFOtelCounterOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_counter(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_COUNTER_H_
