#ifndef CONGELADO_C_OTEL_METER_H_
#define CONGELADO_C_OTEL_METER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/otel/counter.h"
#include "include/c/extern/otel/histogram.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFOtelMeter
    {
        void* plugin_data;
    } TFOtelMeter;

    typedef struct TFOtelMeterOps
    {
        size_t struct_size;
        void (*destroy)(TFOtelMeter* meter);
        void (*get_name)(TFOtelMeter* meter, TF_String* out_name);

        void (*create_counter)(
            TFOtelMeter* meter,
            const TF_String* name,
            const TF_String* description,
            const TF_String* unit,
            TFOtelCounter* out_counter,
            TF_Status* out_status
        );
        void (*create_histogram)(
            TFOtelMeter* meter,
            const TF_String* name,
            const TF_String* description,
            const TF_String* unit,
            TFOtelHistogram* out_histogram,
            TF_Status* out_status
        );
    } TFOtelMeterOps;

#define TF_OTEL_METER_STRUCT_SIZE TF_OFFSET_OF_END(TFOtelMeterOps, create_histogram)

    TF_CAPI_EXPORT void
    create_otel_meter(TFOtelMeterOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_meter(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_METER_H_
