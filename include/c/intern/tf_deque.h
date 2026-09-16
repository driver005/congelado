#ifndef TENSORFLOW_C_TF_DEQUE_H_
#define TENSORFLOW_C_TF_DEQUE_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Deque — plugin vtable for a type-erased double-ended growable collection (std::deque<T> equivalent), fixed to element_size bytes per element at creation.
    typedef struct TF_DequeOps TF_DequeOps;

    typedef struct TF_Deque
    {
        void* plugin_data;
        const TF_DequeOps* ops;
    } TF_Deque;

    // Plugin-facing vtable registered via create_deque.
    typedef struct TF_DequeOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_Deque* deque, size_t element_size);

        // Copy one element_size-byte element from value onto the front.
        void (*push_front)(TF_Deque* deque, const void* value);

        // Copy one element_size-byte element from value onto the back.
        void (*push_back)(TF_Deque* deque, const void* value);

        // Remove the front element. No-op if empty.
        void (*pop_front)(TF_Deque* deque);

        // Remove the back element. No-op if empty.
        void (*pop_back)(TF_Deque* deque);

        // Non-owning pointer to the element at index; NULL if out of range.
        const void* (*get)(const TF_Deque* deque, size_t index);

        // Current element count.
        size_t (*size)(const TF_Deque* deque);

        void (*destroy)(TF_Deque* deque);

    } TF_DequeOps;

#define TF_DEQUE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DequeOps, destroy)

    TF_CAPI_EXPORT void create_deque(TF_DequeOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_deque(void* plugin_context);

    // Real implementation, not declared-only — calls create_deque and fills in deque->ops.
    static inline void init_deque(TF_Deque* deque, TF_Status* status)
    {
        TF_DequeOps* ops = NULL;
        create_deque(&ops, &deque->plugin_data, status);
        deque->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_DEQUE_H_
