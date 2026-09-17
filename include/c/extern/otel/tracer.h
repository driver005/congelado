#ifndef CONGELADO_C_OTEL_TRACER_H_
#define CONGELADO_C_OTEL_TRACER_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/extern/otel/span.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFOtelTracer
    {
        void* plugin_data;
    } TFOtelTracer;

    typedef struct TFOtelTracerOps
    {
        size_t struct_size;
        void (*destroy)(TFOtelTracer* tracer);
        void (*get_name)(TFOtelTracer* tracer, TF_String* out_name);

        void (*start_span)(
            TFOtelTracer* tracer,
            const TF_String* name,
            int kind,
            TFOtelSpan* out_span,
            TF_Status* out_status
        );
    } TFOtelTracerOps;

#define TF_OTEL_TRACER_STRUCT_SIZE TF_OFFSET_OF_END(TFOtelTracerOps, start_span)

    TF_CAPI_EXPORT void
    create_otel_tracer(TFOtelTracerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_tracer(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_TRACER_H_
