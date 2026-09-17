#ifndef CONGELADO_C_OTEL_HISTOGRAM_H_
#define CONGELADO_C_OTEL_HISTOGRAM_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFOtelHistogram
    {
        void* plugin_data;
    } TFOtelHistogram;

    typedef struct TFOtelHistogramOps
    {
        size_t struct_size;
        void (*destroy)(TFOtelHistogram* histogram);
        void (*get_name)(TFOtelHistogram* histogram, TF_String* out_name);
        void (*record)(TFOtelHistogram* histogram, double value, TF_Status* out_status);
    } TFOtelHistogramOps;

#define TF_OTEL_HISTOGRAM_STRUCT_SIZE TF_OFFSET_OF_END(TFOtelHistogramOps, record)

    TF_CAPI_EXPORT void
    create_otel_histogram(TFOtelHistogramOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_histogram(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_HISTOGRAM_H_
