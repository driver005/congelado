#ifndef TENSORFLOW_C_TF_ARRAY_H_
#define TENSORFLOW_C_TF_ARRAY_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Array — plugin vtable for a type-erased fixed-size, contiguous
    // collection (std::array<T, N> equivalent). Both element_size and element
    // count are fixed at creation; unlike TF_Vector, it never grows.
    //
    // TF_Array_Handle is an opaque pointer to a plugin-owned array object.
    typedef struct TF_Array_Handle TF_Array_Handle;

    // Plugin-facing vtable registered via init_array.
    typedef struct TF_Array
    {
        size_t struct_size;

        // Allocate a new array of count element_size-byte elements,
        // value-initialized to zero. Must be freed with destroy.
        TF_Array_Handle* (*new_array)(void* plugin_context, size_t element_size, size_t count);

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(const TF_Array_Handle* array, size_t index);

        // Copy one element_size-byte element from value over the element at
        // index.
        void (*set)(TF_Array_Handle* array, size_t index, const void* value);

        // Fixed element count, as given to new_array.
        size_t (*size)(const TF_Array_Handle* array);

        // Non-owning pointer to the contiguous backing storage.
        void* (*data)(TF_Array_Handle* array);

        // Free a handle returned by new_array.
        void (*destroy)(void* plugin_context, TF_Array_Handle* array);

    } TF_Array;

#define TF_ARRAY_STRUCT_SIZE TF_OFFSET_OF_END(TF_Array, destroy)

    TF_CAPI_EXPORT void init_array(TF_Array** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_ARRAY_H_
