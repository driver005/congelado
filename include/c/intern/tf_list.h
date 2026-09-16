#ifndef TENSORFLOW_C_TF_LIST_H_
#define TENSORFLOW_C_TF_LIST_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_List — plugin vtable for a type-erased doubly linked list (std::list<T> equivalent), fixed to element_size bytes per element at creation. Node handles returned by push_front/push_back remain valid for the node's lifetime, independent of other insertions/erasures.
    typedef struct TF_ListOps TF_ListOps;

    typedef struct TF_List
    {
        void* plugin_data;
        const TF_ListOps* ops;
    } TF_List;

    // TF_List_Node is an opaque pointer to one node within a TF_List.
    typedef struct TF_List_Node TF_List_Node;

    // Callback type for iterating over a list's elements in order.
    typedef void (*TF_ListVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via create_list.
    typedef struct TF_ListOps
    {
        size_t struct_size;

        void (*set_element_size)(TF_List* list, size_t element_size);

        // Copy one element_size-byte element from value onto the front, returning a handle to the new node.
        TF_List_Node* (*push_front)(TF_List* list, const void* value);

        // Copy one element_size-byte element from value onto the back, returning a handle to the new node.
        TF_List_Node* (*push_back)(TF_List* list, const void* value);

        // Remove node from the list, invalidating it.
        void (*erase)(TF_List* list, TF_List_Node* node);

        // Call visitor(capture, element) once per element, front to back.
        void (*for_each)(
            const TF_List* list,
            TF_ListVisitor visitor,
            void* capture
        );

        // Current element count.
        size_t (*size)(const TF_List* list);

        void (*destroy)(TF_List* list);

    } TF_ListOps;

#define TF_LIST_STRUCT_SIZE TF_OFFSET_OF_END(TF_ListOps, destroy)

    TF_CAPI_EXPORT void create_list(TF_ListOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_list(void* plugin_context);

    // Real implementation, not declared-only — calls create_list and fills in list->ops.
    static inline void init_list(TF_List* list, TF_Status* status)
    {
        TF_ListOps* ops = NULL;
        create_list(&ops, &list->plugin_data, status);
        list->ops = ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_LIST_H_
