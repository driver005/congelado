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
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    
    

    
    
    
    
    

    // Opens a new server, returning an owned handle (destroy with TF_Protocol_Server_Destroy).
    // NULL on failure (see status).
    
    

    
    
    

    typedef struct TF_Protocol {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        void (*get_bind_host)(void* plugin_context, TF_String* out);
        uint16_t (*get_bind_port)(void* plugin_context);
        void (*get_tls_cert)(void* plugin_context, TF_String* out);
        void (*get_tls_key)(void* plugin_context, TF_String* out);
        void* (*create_server)(void* plugin_context, TF_Status* status);
        void (*server__destroy)(void* server_context);
        void (*server__start)(void* server_context, TF_Status* status);
        void (*server__stop)(void* server_context, TF_Status* status);
        TF_Bool (*server__is_running)(void* server_context, TF_Status* status);
    } TF_Protocol;

    TF_CAPI_EXPORT extern void TF_InitProtocol(TF_Protocol** ops, void** plugin_context, TF_Status* status);

    typedef struct TF_Protocol_Server {
        size_t struct_size;

    } TF_Protocol_Server;

    TF_CAPI_EXPORT extern void TF_InitProtocol_Server(TF_Protocol_Server** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_PROTOCOL_CONTROLLER_H_
