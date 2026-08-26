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

#ifndef CONGELADO_C_IO_REQUEST_H_
#define CONGELADO_C_IO_REQUEST_H_

#include "c/extern/io/enums.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct TP_IO_Response;

    typedef TF_IO_Method (*TP_IO_Request_GetMethodFn)(void* user_data);
    typedef const TF_TString* (*TP_IO_Request_GetPathFn)(void* user_data);
    typedef void (*TP_IO_Request_SetHeaderFn)(
        void* user_data, const TF_TString* name, const TF_TString* value
    );
    typedef void (*TP_IO_Request_SetBodyFn)(void* user_data, const TF_TString* body);

    typedef struct TP_IO_Request
    {
        size_t struct_size;
        void* ext;
        TP_IO_Request_GetMethodFn get_method_cb;
        TP_IO_Request_GetPathFn get_path_cb;
        TP_IO_Request_SetHeaderFn set_header_cb;
        TP_IO_Request_SetBodyFn set_body_cb;
#define TP_IO_REQUEST_STRUCT_SIZE TF_OFFSET_OF_END(TP_IO_Request, set_body_cb)
    } TP_IO_Request;

    // LINT.ThenChange(:io_request_version)


    static inline TP_IO_Request* TP_IORequestNew(void)
    {
        TP_IO_Request* ptr = (TP_IO_Request*)malloc(sizeof(struct TP_IO_Request));
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = sizeof(struct TP_IO_Request);
        ptr->ext = nullptr;
        ptr->get_method_cb = nullptr;
        ptr->get_path_cb = nullptr;
        ptr->set_header_cb = nullptr;
        ptr->set_body_cb = nullptr;
        return ptr;
    }

    static inline void TP_IORequestDelete(TP_IO_Request* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void TP_IORequest_SetStructSize(TP_IO_Request* builder, size_t struct_size)
    {
        builder->struct_size = struct_size;
    }

    static inline void TP_IORequest_SetExt(TP_IO_Request* builder, void* ext)
    {
        builder->ext = ext;
    }

    static inline void TP_IORequest_SetGetMethodCallback(
        TP_IO_Request* builder, TP_IO_Request_GetMethodFn get_method_cb
    )
    {
        builder->get_method_cb = get_method_cb;
    }

    static inline void
    TP_IORequest_SetGetPathCallback(TP_IO_Request* builder, TP_IO_Request_GetPathFn get_path_cb)
    {
        builder->get_path_cb = get_path_cb;
    }

    static inline void TP_IORequest_SetSetHeaderCallback(
        TP_IO_Request* builder, TP_IO_Request_SetHeaderFn set_header_cb
    )
    {
        builder->set_header_cb = set_header_cb;
    }

    static inline void
    TP_IORequest_SetSetBodyCallback(TP_IO_Request* builder, TP_IO_Request_SetBodyFn set_body_cb)
    {
        builder->set_body_cb = set_body_cb;
    }


#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_IO_REQUEST_H_
