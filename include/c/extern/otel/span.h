#ifndef CONGELADO_C_OTEL_SPAN_H_
#define CONGELADO_C_OTEL_SPAN_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Otel_Span
    {
        void* plugin_data;
    } TF_Otel_Span;

    typedef struct TF_Otel_Span
    {
        size_t struct_size;
        void (*destroy)(TF_Otel_Span* span);
        void (*get_name)(TF_Otel_Span* span, TF_String* out_name);

        void (*set_attribute)(
            TF_Otel_Span* span,
            const TF_String* key,
            const TF_String* value,
            TF_Status* out_status
        );
        void (*set_status)(
            TF_Otel_Span* span,
            int status_code,
            const TF_String* description,
            TF_Status* out_status
        );
        void (*end)(TF_Otel_Span* span, TF_Status* out_status);
    } TF_Otel_SpanOps;

#define TF_OTEL_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TF_Otel_SpanOps, end)

    TF_CAPI_EXPORT void
    create_otel_span(TF_Otel_SpanOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_otel_span(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_OTEL_SPAN_H_
