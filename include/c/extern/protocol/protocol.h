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
#ifndef CONGELADO_C_PROTOCOL_CONTROLLER_H_
#define CONGELADO_C_PROTOCOL_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif


    // Opens a new server, returning an owned handle (destroy with server_destroy).
    // NULL on failure (see status).


    // --------------------------------------------------------------------------
    // Error channel: TF_Status* is the sole error channel for every fallible slot.
    // For value-returning slots (bool, ...) the returned value is the query result
    // and is meaningful only when status is OK; on failure the value is unspecified
    // and the error lives in *status. Slots with no TF_Status* parameter are pure
    // accessors that cannot fail.
    // --------------------------------------------------------------------------

    typedef struct TF_Protocol_Server TF_Protocol_Server;

    typedef struct TF_Protocol
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*get_bind_host)(void* plugin_context, TF_String* out);
        uint16_t (*get_bind_port)(void* plugin_context);
        void (*get_tls_cert)(void* plugin_context, TF_String* out);
        void (*get_tls_key)(void* plugin_context, TF_String* out);
        TF_Protocol_Server* (*create_server)(void* plugin_context, TF_Status* status);
        void (*server_destroy)(TF_Protocol_Server* server_context);
        void (*server_start)(TF_Protocol_Server* server_context, TF_Status* status);
        void (*server_stop)(TF_Protocol_Server* server_context, TF_Status* status);
        bool (*server_is_running)(TF_Protocol_Server* server_context, TF_Status* status);
    } TF_Protocol;

#define TF_PROTOCOL_STRUCT_SIZE TF_OFFSET_OF_END(TF_Protocol, server_is_running)

    TF_CAPI_EXPORT void init_protocol(TF_Protocol** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_PROTOCOL_CONTROLLER_H_
