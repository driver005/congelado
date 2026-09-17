#ifndef CONGELADO_C_OTEL_HISTOGRAM_H_
#define CONGELADO_C_OTEL_HISTOGRAM_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Otel_Histogram
    {
        void* plugin_data;
    } TF_Otel_Histogram;

    typedef struct TF_Otel_HistogramOps
    {
        size_t struct_size;
        void (*destroy)(TF_Otel_Histogram* histogram);
        void (*get_name)(TF_Otel_Histogram* histogram, TF_String* out_name);
        void (*record)(TF_Otel_Histogram* histogram, double value, TF_Status* out_status);
    } TF_Otel_HistogramOps;

#define TF_OTEL_HISTOGRAM_STRUCT_SIZE TF_OFFSET_OF_END(TF_Otel_HistogramOps, record)

    TF_CAPI_EXPORT void
    create_otel_histogram(TF_Otel_HistogramOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_histogram(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_HISTOGRAM_H_
