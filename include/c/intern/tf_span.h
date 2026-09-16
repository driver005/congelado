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

        const void* (*get)(const TF_Span* span, size_t index);
        size_t (*size)(const TF_Span* span);
        void* (*data)(const TF_Span* span);
        TF_Span* (*subspan)(const TF_Span* span, size_t offset, size_t count);
        void (*destroy)(TF_Span* span);

    } TF_SpanOps;

#define TF_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TF_SpanOps, destroy)

    TF_CAPI_EXPORT void create_span(TF_SpanOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_span(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SPAN_H_
