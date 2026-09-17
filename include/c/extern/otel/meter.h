#ifndef CONGELADO_C_OTEL_METER_H_
#define CONGELADO_C_OTEL_METER_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/otel/counter.h"
#include "c/extern/otel/histogram.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Otel_Meter
    {
        void* plugin_data;
    } TF_Otel_Meter;

    typedef struct TF_Otel_MeterOps
    {
        size_t struct_size;
        void (*destroy)(TF_Otel_Meter* meter);
        void (*get_name)(TF_Otel_Meter* meter, TF_String* out_name);

        void (*create_counter)(
            TF_Otel_Meter* meter,
            const TF_String* name,
            const TF_String* description,
            const TF_String* unit,
            TF_Otel_Counter* out_counter,
            TF_Status* out_status
        );
        void (*create_histogram)(
            TF_Otel_Meter* meter,
            const TF_String* name,
            const TF_String* description,
            const TF_String* unit,
            TF_Otel_Histogram* out_histogram,
            TF_Status* out_status
        );
    } TF_Otel_MeterOps;

#define TF_OTEL_METER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Otel_MeterOps, create_histogram)

    TF_CAPI_EXPORT void
    create_otel_meter(TF_Otel_MeterOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_meter(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_METER_H_
