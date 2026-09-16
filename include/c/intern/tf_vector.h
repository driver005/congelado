#ifndef TENSORFLOW_C_TF_VECTOR_H_
#define TENSORFLOW_C_TF_VECTOR_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Vector — plugin vtable for a type-erased growable, contiguous
    // collection (std::vector<T> equivalent), fixed to element_size bytes per
    // element at creation.
    //
    // TF_Vector_Handle is an opaque pointer to a plugin-owned vector object.
    typedef struct TF_Vector_Handle TF_Vector_Handle;

    // Plugin-facing vtable registered via init_vector.
    typedef struct TF_Vector
    {
        size_t struct_size;

        // Allocate a new, empty vector of element_size-byte elements. Must be
        // freed with destroy.
        TF_Vector_Handle* (*new_vector)(void* plugin_context, size_t element_size);

        // Copy one element_size-byte element from value onto the end.
        void (*push_back)(TF_Vector_Handle* vector, const void* value);

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(const TF_Vector_Handle* vector, size_t index);

        // Copy one element_size-byte element from value over the element at
        // index.
        void (*set)(
            TF_Vector_Handle* vector,
            size_t index,
            const void* value
        );

        // Current element count.
        size_t (*size)(const TF_Vector_Handle* vector);

        // Current storage capacity in elements.
        size_t (*capacity)(const TF_Vector_Handle* vector);

        // Ensure capacity for at least new_capacity elements.
        void (*reserve)(TF_Vector_Handle* vector, size_t new_capacity);

        // Non-owning pointer to the contiguous backing storage.
        void* (*data)(TF_Vector_Handle* vector);

        // Free a handle returned by new_vector.
        void (*destroy)(void* plugin_context, TF_Vector_Handle* vector);

    } TF_Vector;

#define TF_VECTOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_Vector, destroy)

    TF_CAPI_EXPORT void init_vector(TF_Vector** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_VECTOR_H_
