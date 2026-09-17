#ifndef TENSORFLOW_C_TF_SPAN_H_
#define TENSORFLOW_C_TF_SPAN_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Span
    {
        void* plugin_data;
    } TF_Span;

    typedef struct TF_SpanOps
    {
        size_t struct_size;

        void (*get)(const TF_Span* span, size_t index, const void** out_value, TF_Status* out_status);
        void (*size)(const TF_Span* span, size_t* out_size);
        void (*data)(const TF_Span* span, void** out_data);
        void (*subspan)(const TF_Span* span, size_t offset, size_t count, TF_Span* out_span, TF_Status* out_status);
        void (*destroy)(TF_Span* span);

    } TF_SpanOps;

#define TF_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TF_SpanOps, destroy)

    TF_CAPI_EXPORT void create_span(TF_SpanOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_span(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SPAN_H_
