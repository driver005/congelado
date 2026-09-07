#ifndef TENSORFLOW_C_TF_LIST_H_
#define TENSORFLOW_C_TF_LIST_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_List — plugin vtable for a type-erased doubly linked list
    // (std::list<T> equivalent), fixed to element_size bytes per element at
    // creation. Node handles returned by push_front/push_back remain valid
    // for the node's lifetime, independent of other insertions/erasures.
    //
    // TF_List_Handle is an opaque pointer to a plugin-owned list object.
    typedef struct TF_List_Handle TF_List_Handle;

    // TF_List_Node is an opaque pointer to one node within a TF_List_Handle.
    typedef struct TF_List_Node TF_List_Node;

    // Callback type for iterating over a list's elements in order.
    typedef void (*TF_ListVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via init_list.
    typedef struct TF_List
    {
        size_t struct_size;

        // Allocate a new, empty list of element_size-byte elements. Must be
        // freed with destroy.
        TF_List_Handle* (*new_list)(void* plugin_context, size_t element_size);

        // Copy one element_size-byte element from value onto the front,
        // returning a handle to the new node.
        TF_List_Node* (*push_front)(void* plugin_context, TF_List_Handle* list, const void* value);

        // Copy one element_size-byte element from value onto the back,
        // returning a handle to the new node.
        TF_List_Node* (*push_back)(void* plugin_context, TF_List_Handle* list, const void* value);

        // Remove node from the list, invalidating it.
        void (*erase)(void* plugin_context, TF_List_Handle* list, TF_List_Node* node);

        // Call visitor(capture, element) once per element, front to back.
        void (*for_each)(
            void* plugin_context,
            const TF_List_Handle* list,
            TF_ListVisitor visitor,
            void* capture
        );

        // Current element count.
        size_t (*size)(void* plugin_context, const TF_List_Handle* list);

        // Free a handle returned by new_list.
        void (*destroy)(void* plugin_context, TF_List_Handle* list);

    } TF_List;

#define TF_LIST_STRUCT_SIZE TF_OFFSET_OF_END(TF_List, destroy)

    TF_CAPI_EXPORT void init_list(TF_List** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_LIST_H_
