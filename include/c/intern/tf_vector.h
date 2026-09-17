#ifndef TENSORFLOW_C_TF_VECTOR_H_
#define TENSORFLOW_C_TF_VECTOR_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Vector — plugin vtable for a type-erased growable, contiguous collection (std::vector<T> equivalent), fixed to element_size bytes per element at creation.

    typedef struct TF_Vector
    {
        void* plugin_data;
    } TF_Vector;

    // Plugin-facing vtable registered via create_vector.
    typedef struct TF_VectorOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_Vector* vector, size_t element_size);

        // Copy one element_size-byte element from value onto the end.
        void (*push_back)(TF_Vector* vector, const void* value);

        // Non-owning pointer to the element at index; NULL if out of range.
        void (*get)(const TF_Vector* vector, size_t index, const void** out_value, TF_Status* out_status);

        // Copy one element_size-byte element from value over the element at index.
        void (*set)(
            TF_Vector* vector,
            size_t index,
            const void* value,
            TF_Status* out_status
        );

        // Current element count.
        void (*size)(const TF_Vector* vector, size_t* out_size);

        // Current storage capacity in elements.
        void (*capacity)(const TF_Vector* vector, size_t* out_capacity);

        // Ensure capacity for at least new_capacity elements.
        void (*reserve)(TF_Vector* vector, size_t new_capacity, TF_Status* out_status);

        // Non-owning pointer to the contiguous backing storage.
        void (*data)(TF_Vector* vector, void** out_data);

        void (*destroy)(TF_Vector* vector);

    } TF_VectorOps;

#define TF_VECTOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_VectorOps, destroy)

    TF_CAPI_EXPORT void create_vector(TF_VectorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_vector(void* plugin_context);

    // Real implementation, not declared-only — calls create_vector
    static inline void init_vector(TF_Vector* vector, TF_Status* out_status)
    {
        TF_VectorOps* ops = NULL;
        create_vector(&ops, &vector->plugin_data, out_status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_VECTOR_H_
