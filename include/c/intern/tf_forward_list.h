#ifndef TENSORFLOW_C_TF_FORWARD_LIST_H_
#define TENSORFLOW_C_TF_FORWARD_LIST_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_ForwardList — plugin vtable for a type-erased singly linked list (std::forward_list<T> equivalent), fixed to element_size bytes per element at creation.
    typedef struct TF_ForwardListOps TF_ForwardListOps;

    typedef struct TF_ForwardList
    {
        void* plugin_data;
        const TF_ForwardListOps* ops;
    } TF_ForwardList;

    // TF_ForwardList_Node is an opaque pointer to one node within a TF_ForwardList.
    typedef struct TF_ForwardList_Node TF_ForwardList_Node;

    // Callback type for iterating over a forward_list's elements in order.
    typedef void (*TF_ForwardListVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via create_forward_list.
    typedef struct TF_ForwardListOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_ForwardList* list, size_t element_size);

        // Copy one element_size-byte element from value onto the front, returning a handle to the new node.
        TF_ForwardList_Node* (*push_front)(
            TF_ForwardList* list,
            const void* value
        );

        // Erase the node immediately following node (or the front node, if node is NULL). Matches std::forward_list::erase_after semantics.
        void (*erase_after)(
            TF_ForwardList* list,
            TF_ForwardList_Node* node
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
    create_forward_list(TF_ForwardListOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_forward_list(void* plugin_context);

    // Real implementation, not declared-only — calls create_forward_list and fills in list->ops.
    static inline void init_forward_list(TF_ForwardList* list, TF_Status* status)
    {
        TF_ForwardListOps* ops = NULL;
        create_forward_list(&ops, &list->plugin_data, status);
        list->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_FORWARD_LIST_H_
