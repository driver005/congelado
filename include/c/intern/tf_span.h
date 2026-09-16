#ifndef TENSORFLOW_C_TF_SPAN_H_
#define TENSORFLOW_C_TF_SPAN_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Span — plugin vtable for a type-erased non-owning view over contiguous memory (std::span<T> equivalent), fixed to element_size bytes per element at creation. A span never allocates, copies, or frees the memory it views; destroy only frees the lightweight handle itself.
    //
    // TF_Span is an opaque pointer to a plugin-owned span object.
    typedef struct TF_Span
    {
        void* plugin_data;
    } TF_Span;

    // Plugin-facing vtable registered via create_span.
    typedef struct TF_SpanOps
    {
        size_t struct_size;

        // Wrap an existing, caller-owned block of count element_size-byte elements starting at data. The block must outlive the span. Must be freed with destroy.
        TF_Span* (*new_span)(
            void* plugin_context,
            void* data,
            size_t count,
            size_t element_size
        );

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(const TF_Span* span, size_t index);

        // Element count.
        size_t (*size)(const TF_Span* span);

        // Non-owning pointer to the viewed contiguous storage.
        void* (*data)(const TF_Span* span);

        // A new span viewing the same storage, starting at offset for count elements. Must be freed independently with destroy.
        TF_Span* (*subspan)(
            const TF_Span* span,
            size_t offset,
            size_t count
        );

        // Free a handle returned by new_span or subspan. Never touches the viewed memory.
        void (*destroy)(TF_Span* span);

    } TF_SpanOps;

#define TF_SPAN_STRUCT_SIZE TF_OFFSET_OF_END(TF_SpanOps, destroy)

    TF_CAPI_EXPORT void create_span(TF_SpanOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_span(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_SPAN_H_
