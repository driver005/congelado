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
#ifndef CONGELADO_C_DATABASE_H_
#define CONGELADO_C_DATABASE_H_

#include "c/abi/macros.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

// --------------------------------------------------------------------------
// C API for Database. The API allows registering a pluggable database backend
// with Congelado.
//
// Conventions:
//   * TF_: Set/filled by core, unless marked otherwise.
//   * TP_: Set/filled by plug-in, unless marked otherwise.
//   * Structs begin with size_t struct_size and void* ext.

#ifdef __cplusplus
extern "C"
{
#endif

    // Callback for async database operations.
    typedef void (*TF_Database_CompletionFn)(const TF_TString* result, void* user_data);

    // --------------------------------------------------------------------------
    // Database callback types — defined separately for use in struct and C++ module.
    typedef TF_Bool (*TP_Database_IsConnectedFn)(void* user_data);
    typedef void (*TP_Database_QueryFn)(
        void* user_data,
        const TF_TString* payload,
        TF_Database_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Database_InsertFn)(
        void* user_data,
        const TF_TString* payload,
        TF_Database_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Database_UpdateFn)(
        void* user_data,
        const TF_TString* payload,
        TF_Database_CompletionFn completion,
        void* cb_user_data
    );
    typedef void (*TP_Database_RemoveFn)(
        void* user_data,
        const TF_TString* payload,
        TF_Database_CompletionFn completion,
        void* cb_user_data
    );

    // Forward declaration — TP_Database is fully defined below; needed here for
    // TF_DatabaseRegistrationParams_DestroyDatabase which references TP_Database*.
    typedef struct TP_Database TP_Database;

    typedef void (*TF_DatabaseRegistrationParams_DestroyDatabase)(TP_Database* db);

    // --------------------------------------------------------------------------
    // TP_Database holds database configuration and user callbacks.
    typedef struct TP_Database
    {
        size_t struct_size;
        void* ext; // free-form data set by plugin.

        // Backend name (small-string-optimized).
        TF_TString backend_name;

        // User callbacks.
        TP_Database_IsConnectedFn is_connected_cb;
        TP_Database_QueryFn query_cb;
        TP_Database_InsertFn insert_cb;
        TP_Database_UpdateFn update_cb;
        TP_Database_RemoveFn remove_cb;

        // The struct size must be updated when adding new members.
#define TP_DATABASE_STRUCT_SIZE TF_OFFSET_OF_END(TP_Database, remove_cb)
    } TP_Database;

    // --------------------------------------------------------------------------
    // TF_DatabaseRegistrationParams holds the pointers to TP_Database.
    typedef struct TF_DatabaseRegistrationParams
    {
        size_t struct_size;
        void* ext; // reserved for future use

        // Database C API version.
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;

        // [in/out] Memory owned by core but attributes within are populated by the
        // plugin.
        TP_Database* db;

        // [out] Pointer to plugin's cleanup function.
        TF_DatabaseRegistrationParams_DestroyDatabase destroy_database;

        // The struct size must be updated when adding new members.
#define TF_DATABASE_REGISTRATION_PARAMS_STRUCT_SIZE                                                \
    TF_OFFSET_OF_END(TF_DatabaseRegistrationParams, destroy_database)
    } TF_DatabaseRegistrationParams;

#define DB_MAJOR 0
#define DB_MINOR 0
#define DB_PATCH 1

    // TF_InitDatabase is used to do database registration.
    void TF_InitDatabase(TF_DatabaseRegistrationParams* params, TF_Status* status);

    // TP_DatabaseAlloc — allocates and initializes a TP_Database struct.


    static inline TP_Database* TP_DatabaseNew(void)
    {
        TP_Database* ptr = (TP_Database*)malloc(TP_DATABASE_STRUCT_SIZE);
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = TP_DATABASE_STRUCT_SIZE;
        ptr->ext = nullptr;
        TF_StringInit(&ptr->backend_name);
        ptr->is_connected_cb = nullptr;
        ptr->query_cb = nullptr;
        ptr->insert_cb = nullptr;
        ptr->update_cb = nullptr;
        ptr->remove_cb = nullptr;
        return ptr;
    }

    // TP_DatabaseDealloc — frees a TP_Database allocated by TP_DatabaseAlloc.
    static inline void TP_DatabaseDelete(TP_Database* ptr)
    {
        if (!ptr) {
            return;
        }
        TF_StringDealloc(&ptr->backend_name);
        free(ptr);
    }

    static inline void TP_Database_SetIsConnectedCallback(
        TP_Database* builder, TP_Database_IsConnectedFn is_connected_cb
    )
    {
        builder->is_connected_cb = is_connected_cb;
    }

    static inline void
    TP_Database_SetQueryCallback(TP_Database* builder, TP_Database_QueryFn query_cb)
    {
        builder->query_cb = query_cb;
    }

    static inline void
    TP_Database_SetInsertCallback(TP_Database* builder, TP_Database_InsertFn insert_cb)
    {
        builder->insert_cb = insert_cb;
    }

    static inline void
    TP_Database_SetUpdateCallback(TP_Database* builder, TP_Database_UpdateFn update_cb)
    {
        builder->update_cb = update_cb;
    }

    static inline void
    TP_Database_SetRemoveCallback(TP_Database* builder, TP_Database_RemoveFn remove_cb)
    {
        builder->remove_cb = remove_cb;
    }


#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_DATABASE_H_
