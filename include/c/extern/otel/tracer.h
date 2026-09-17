#ifndef CONGELADO_C_OTEL_TRACER_H_
#define CONGELADO_C_OTEL_TRACER_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"
#include "c/extern/otel/span.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Otel_Tracer
    {
        void* plugin_data;
    } TF_Otel_Tracer;

    typedef struct TF_Otel_TracerOps
    {
        size_t struct_size;
        void (*destroy)(TF_Otel_Tracer* tracer);
        void (*get_name)(TF_Otel_Tracer* tracer, TF_String* out_name);

        void (*start_span)(
            TF_Otel_Tracer* tracer,
            const TF_String* name,
            int kind,
            TF_Otel_Span* out_span,
            TF_Status* out_status
        );
    } TF_Otel_TracerOps;

#define TF_OTEL_TRACER_STRUCT_SIZE TF_OFFSET_OF_END(TF_Otel_TracerOps, start_span)

    TF_CAPI_EXPORT void
    create_otel_tracer(TF_Otel_TracerOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_tracer(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_TRACER_H_
