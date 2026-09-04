/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_C_TF_FORWARD_LIST_H_
#define TENSORFLOW_C_TF_FORWARD_LIST_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_ForwardList — plugin vtable for a type-erased singly linked list
    // (std::forward_list<T> equivalent), fixed to element_size bytes per
    // element at creation.
    //
    // TF_ForwardList_Handle is an opaque pointer to a plugin-owned list.
    typedef struct TF_ForwardList_Handle TF_ForwardList_Handle;

    // TF_ForwardList_Node is an opaque pointer to one node within a
    // TF_ForwardList_Handle.
    typedef struct TF_ForwardList_Node TF_ForwardList_Node;

    // Callback type for iterating over a forward_list's elements in order.
    typedef void (*TF_ForwardListVisitor)(void* capture, const void* element);

    // Plugin-facing vtable registered via init_forward_list.
    typedef struct TF_ForwardList
    {
        size_t struct_size;

        // Allocate a new, empty forward_list of element_size-byte elements.
        // Must be freed with destroy.
        TF_ForwardList_Handle* (*new_forward_list)(void* plugin_context, size_t element_size);

        // Copy one element_size-byte element from value onto the front,
        // returning a handle to the new node.
        TF_ForwardList_Node* (*push_front)(
            void* plugin_context,
            TF_ForwardList_Handle* list,
            const void* value
        );

        // Erase the node immediately following node (or the front node, if
        // node is NULL). Matches std::forward_list::erase_after semantics.
        void (*erase_after)(
            void* plugin_context,
            TF_ForwardList_Handle* list,
            TF_ForwardList_Node* node
        );

        // Call visitor(capture, element) once per element, front to back.
        void (*for_each)(
            void* plugin_context,
            const TF_ForwardList_Handle* list,
            TF_ForwardListVisitor visitor,
            void* capture
        );

        // Free a handle returned by new_forward_list.
        void (*destroy)(void* plugin_context, TF_ForwardList_Handle* list);

    } TF_ForwardList;

#define TF_FORWARD_LIST_STRUCT_SIZE TF_OFFSET_OF_END(TF_ForwardList, destroy)

    TF_CAPI_EXPORT void
    init_forward_list(TF_ForwardList** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_FORWARD_LIST_H_
