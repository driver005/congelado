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

#ifndef CONGELADO_C_IO_RESPONSE_H_
#define CONGELADO_C_IO_RESPONSE_H_

#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TP_IO_Response_SetStatusFn)(void* user_data, int32_t status_code);
    typedef void (*TP_IO_Response_SetHeaderFn)(
        void* user_data, const TF_TString* name, const TF_TString* value
    );
    typedef void (*TP_IO_Response_SetBodyFn)(void* user_data, const TF_TString* body);

    typedef struct TP_IO_Response
    {
        size_t struct_size;
        void* ext;
        TP_IO_Response_SetStatusFn set_status_cb;
        TP_IO_Response_SetHeaderFn set_header_cb;
        TP_IO_Response_SetBodyFn set_body_cb;
#define TP_IO_RESPONSE_STRUCT_SIZE TF_OFFSET_OF_END(TP_IO_Response, set_body_cb)
    } TP_IO_Response;

    // LINT.ThenChange(:io_response_version)


    static inline TP_IO_Response* TP_IOResponseNew(void)
    {
        TP_IO_Response* ptr = (TP_IO_Response*)malloc(sizeof(struct TP_IO_Response));
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = sizeof(struct TP_IO_Response);
        ptr->ext = nullptr;
        ptr->set_status_cb = nullptr;
        ptr->set_header_cb = nullptr;
        ptr->set_body_cb = nullptr;
        return ptr;
    }

    static inline void TP_IOResponseDelete(TP_IO_Response* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void TP_IOResponse_SetStructSize(TP_IO_Response* builder, size_t struct_size)
    {
        builder->struct_size = struct_size;
    }

    static inline void TP_IOResponse_SetExt(TP_IO_Response* builder, void* ext)
    {
        builder->ext = ext;
    }

    static inline void TP_IOResponse_SetSetStatusCallback(
        TP_IO_Response* builder, TP_IO_Response_SetStatusFn set_status_cb
    )
    {
        builder->set_status_cb = set_status_cb;
    }

    static inline void TP_IOResponse_SetSetHeaderCallback(
        TP_IO_Response* builder, TP_IO_Response_SetHeaderFn set_header_cb
    )
    {
        builder->set_header_cb = set_header_cb;
    }

    static inline void
    TP_IOResponse_SetSetBodyCallback(TP_IO_Response* builder, TP_IO_Response_SetBodyFn set_body_cb)
    {
        builder->set_body_cb = set_body_cb;
    }


#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_IO_RESPONSE_H_
