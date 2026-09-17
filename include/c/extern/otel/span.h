#ifndef CONGELADO_C_OTEL_SPAN_H_
#define CONGELADO_C_OTEL_SPAN_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TFOtelSpan
    {
        void* plugin_data;
    } TFOtelSpan;

    typedef struct TFOtelSpanOps
    {
        size_t struct_size;
        void (*destroy)(TFOtelSpan* span);
        void (*get_name)(TFOtelSpan* span, TF_String* out_name);

        void (*set_attribute)(
            TFOtelSpan* span,
            const TF_String* key,
            const TF_String* value,
            TF_Status* out_status
        );
        void (*set_status)(
            TFOtelSpan* span,
            int status_code,
            const TF_String* description,
            TF_Status* out_status
        );
        void (*end)(TFOtelSpan* span, TF_Status* out_status);
    } TFOtelSpanOps;

#define TF_OTEL_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TFOtelSpanOps, end)

    TF_CAPI_EXPORT void
    create_otel_span(TFOtelSpanOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_span(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_SPAN_H_
