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

#ifndef CONGELADO_C_IO_REGISTRATION_H_
#define CONGELADO_C_IO_REGISTRATION_H_

#include "c/extern/io/opaque_types.h"
#include "c/extern/io/request.h"
#include "c/extern/io/response.h"
#include "c/intern/tf_bool.h"
#include "c/intern/tf_status.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
typedef struct TP_IO_Request TP_IO_Request;
typedef struct TP_IO_Response TP_IO_Response;
typedef struct TP_IO TP_IO;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef TP_IO_Request* (*TP_IO_CreateRequestFn)(void* user_data, TF_Status* status);
    typedef TP_IO_Response* (*TP_IO_CreateResponseFn)(void* user_data, TF_Status* status);

    typedef void (*TF_IoRegistrationParams_DestroyIo)(TP_IO* io);
    typedef void (*TF_IoRegistrationParams_DestroyRequest)(TP_IO_Request* request);
    typedef void (*TF_IoRegistrationParams_DestroyResponse)(TP_IO_Response* response);

    typedef struct TP_IO
    {
        size_t struct_size;
        void* ext;
        TP_IO_CreateRequestFn create_request_cb;
        TP_IO_CreateResponseFn create_response_cb;
#define TP_IO_STRUCT_SIZE TF_OFFSET_OF_END(TP_IO, create_response_cb)
    } TP_IO;

    typedef struct TF_IoRegistrationParams
    {
        size_t struct_size;
        void* ext;
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;
        TP_IO* io;
        TF_IoRegistrationParams_DestroyIo destroy_io;
        TF_IoRegistrationParams_DestroyRequest destroy_request;
        TF_IoRegistrationParams_DestroyResponse destroy_response;
#define TF_IO_REGISTRATION_PARAMS_STRUCT_SIZE                                                      \
    TF_OFFSET_OF_END(TF_IoRegistrationParams, destroy_response)
    } TF_IoRegistrationParams;

    static inline TP_IO* TP_IONew(void)
    {
        TP_IO* ptr = (TP_IO*)malloc(sizeof(struct TP_IO));
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = sizeof(struct TP_IO);
        ptr->ext = nullptr;
        ptr->create_request_cb = nullptr;
        ptr->create_response_cb = nullptr;
        return ptr;
    }

    static inline void TP_IODelete(TP_IO* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void
    TP_IO_SetCreateRequestCallback(TP_IO* builder, TP_IO_CreateRequestFn create_request_cb)
    {
        builder->create_request_cb = create_request_cb;
    }

    static inline void
    TP_IO_SetCreateResponseCallback(TP_IO* builder, TP_IO_CreateResponseFn create_response_cb)
    {
        builder->create_response_cb = create_response_cb;
    }

#define IO_MAJOR 0
#define IO_MINOR 0
#define IO_PATCH 1

    void TF_InitIo(TF_IoRegistrationParams* params, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_IO_REGISTRATION_H_
