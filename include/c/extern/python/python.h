/* Copyright 2024 The Congelado Authors. All Rights Reserved.

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
#ifndef CONGELADO_C_PYTHON_H_
#define CONGELADO_C_PYTHON_H_

#include "c/abi/api.h"
#include "c/abi/macros.h"
#include "c/intern/tf_buffer.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // TF_Python — plugin vtable for the TensorFlow graph/session mutation helpers the
    // Python API depends on (api.h's "python c api" section). The mainframe drives
    // TF graph construction through these slots; a plugin backend implements them
    // against the TensorFlow C API.
    typedef struct TF_Python
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);

        // Add control input to `op`.
        void (*add_control_input)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Operation* op,
            TF_Operation* input
        );

        // Change an attr value in the node_def protocol buffer.
        void (*set_attr)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Operation* op,
            const TF_TString* attr_name,
            TF_Buffer_Data* attr_value_proto,
            TF_Status* status
        );

        // Clear the attr in the node_def protocol buffer.
        void (*clear_attr)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Operation* op,
            const TF_TString* attr_name,
            TF_Status* status
        );

        // Set the experimental_type field in the node_def protocol buffer.
        void (*set_full_type)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Operation* op,
            const TF_Buffer_Data* full_type_proto
        );

        // Set the requested device for `op`.
        void (*set_requested_device)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Operation* op,
            const TF_TString* device
        );

        // Update `dst` to consume `new_src`.
        void (*update_edge)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Output new_src,
            TF_Input dst,
            TF_Status* status
        );

        // Extend `session` with any new operations added to its associated graph.
        void (*extend_session)(
            void* plugin_context,
            TF_Session* session,
            TF_Status* status
        );

        // Return the serialized HandleData proto for `output`, or NULL on failure
        // (the caller frees the returned buffer).
        TF_Buffer_Data* (*get_handle_shape_and_type)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Output output
        );

        // Set `output`'s HandleData from a serialized proto.
        void (*set_handle_shape_and_type)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Output output,
            const void* proto,
            size_t proto_len,
            TF_Status* status
        );

        // Add a new input edge to the While op `dst`.
        void (*add_while_input_hack)(
            void* plugin_context,
            TF_Graph* graph,
            TF_Output new_src,
            TF_Operation* dst,
            TF_Status* status
        );

    } TF_Python;

#define TF_PYTHON_STRUCT_SIZE TF_OFFSET_OF_END(TF_Python, add_while_input_hack)

    TF_CAPI_EXPORT void
    init_python(TF_Python** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_PYTHON_H_
