#ifndef TENSORFLOW_C_TF_ARRAY_H_
#define TENSORFLOW_C_TF_ARRAY_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Array — plugin vtable for a type-erased fixed-size, contiguous collection (std::array<T, N> equivalent). Both element_size and element count are fixed at creation; unlike TF_Vector, it never grows.
    typedef struct TF_ArrayOps TF_ArrayOps;

    typedef struct TF_Array
    {
        void* plugin_data;
        const TF_ArrayOps* ops;
    } TF_Array;

    // Plugin-facing vtable registered via create_array.
    typedef struct TF_ArrayOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_Array* array, size_t element_size);
        void (*set_count)(TF_Array* array, size_t count);

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(const TF_Array* array, size_t index);

        // Copy one element_size-byte element from value over the element at index.
        void (*set)(TF_Array* array, size_t index, const void* value);

        // Element count, as given via set_count.
        size_t (*size)(const TF_Array* array);

        // Non-owning pointer to the contiguous backing storage.
        void* (*data)(TF_Array* array);

        void (*destroy)(TF_Array* array);

    } TF_ArrayOps;

#define TF_ARRAY_STRUCT_SIZE TF_OFFSET_OF_END(TF_ArrayOps, destroy)

    TF_CAPI_EXPORT void create_array(TF_ArrayOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_array(void* plugin_context);

    // Real implementation, not declared-only — calls create_array and fills in array->ops.
    static inline void init_array(TF_Array* array, TF_Status* status)
    {
        TF_ArrayOps* ops = NULL;
        create_array(&ops, &array->plugin_data, status);
        array->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_ARRAY_H_
