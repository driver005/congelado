#ifndef TENSORFLOW_C_TF_FORWARD_LIST_FORWARD_LIST_H_
#define TENSORFLOW_C_TF_FORWARD_LIST_FORWARD_LIST_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/forward_list/node.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_ForwardList — plugin vtable for a type-erased singly linked list (std::forward_list<T> equivalent), fixed to element_size bytes per element at creation.

    typedef struct TF_ForwardList
    {
        void* plugin_data;
    } TF_ForwardList;

    // Callback type for iterating over a forward_list's elements in order.
    typedef void (*TF_ForwardListVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via create_forward_list.
    typedef struct TF_ForwardListOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_ForwardList* list, size_t element_size);

        // Copy one element_size-byte element from value onto the front, returning a handle to the new node.
        void (*push_front)(
            TF_ForwardList* list,
            const void* value,
            TFForwardListNode* out_node,
            TF_Status* out_status
        );

        // Erase the node immediately following node (or the front node, if node is NULL). Matches std::forward_list::erase_after semantics.
        void (*erase_after)(
            TF_ForwardList* list,
            TFForwardListNode* node
        );

        // Call visitor(capture, element) once per element, front to back.
        void (*for_each)(
            const TF_ForwardList* list,
            TF_ForwardListVisitor visitor,
            void* capture
        );

        void (*destroy)(TF_ForwardList* list);

    } TF_ForwardListOps;

#define TF_FORWARD_LIST_STRUCT_SIZE TF_OFFSET_OF_END(TF_ForwardListOps, destroy)

    TF_CAPI_EXPORT void
    create_forward_list(TF_ForwardListOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_forward_list(void* plugin_context);

    // Real implementation, not declared-only — calls create_forward_list
    static inline void init_forward_list(TF_ForwardList* list, TF_Status* out_status)
    {
        TF_ForwardListOps* ops = NULL;
        create_forward_list(&ops, &list->plugin_data, out_status);
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_FORWARD_LIST_FORWARD_LIST_H_
