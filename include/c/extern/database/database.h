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
#ifndef CONGELADO_C_DATABASE_CONTROLLER_H_
#define CONGELADO_C_DATABASE_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Callback for async database operations.
    typedef void (*TF_Database_CompletionFn)(const TF_TString* result, void* user_data);

    // --------------------------------------------------------------------------
    // Error channel: TF_Status* is the sole error channel for every fallible slot.
    // For value-returning slots (bool, int64_t, ...) the returned value is the
    // query result and is meaningful only when status is OK; on failure the value
    // is unspecified and the error lives in *status. Slots with no TF_Status*
    // parameter are pure accessors that cannot fail.
    // --------------------------------------------------------------------------

    typedef struct TF_Database
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        bool (*is_connected)(void* plugin_context, TF_Status* status);
        void (*query)(
            void* plugin_context,
            const TF_TString* payload,
            TF_Database_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*insert)(
            void* plugin_context,
            const TF_TString* payload,
            TF_Database_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*update)(
            void* plugin_context,
            const TF_TString* payload,
            TF_Database_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
        void (*remove)(
            void* plugin_context,
            const TF_TString* payload,
            TF_Database_CompletionFn completion,
            void* cb_user_data,
            TF_Status* status
        );
    } TF_Database;

#define TF_DATABASE_STRUCT_SIZE TF_OFFSET_OF_END(TF_Database, remove)

    TF_CAPI_EXPORT void init_database(TF_Database** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_DATABASE_CONTROLLER_H_
