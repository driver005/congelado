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
#ifndef CONGELADO_C_IO_CONTROLLER_H_
#define CONGELADO_C_IO_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/extern/io/enums.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct TF_IO_Request TF_IO_Request;
    typedef struct TF_IO_Response TF_IO_Response;

    typedef struct TF_IO
    {
        size_t struct_size;
        void (*destroy)(void* plugin_context);
        void (*get_name)(void* plugin_context, TF_String* out);
        TF_IO_Request* (*create_request)(void* plugin_context, TF_Status* status);
        void (*request__destroy)(TF_IO_Request* request_context);
        TF_IO_Response* (*create_response)(void* plugin_context, TF_Status* status);
        void (*response__destroy)(TF_IO_Response* response_context);
        TF_IO_Method (*request__get_method)(TF_IO_Request* request_context);
        void (*request__get_path)(TF_IO_Request* request_context, TF_String* out);
        void (*request__set_header)(
            TF_IO_Request* request_context,
            const TF_TString* name,
            const TF_TString* value,
            TF_Status* status
        );
        void (*request__set_body)(TF_IO_Request* request_context, const TF_TString* body, TF_Status* status);
        void (*response__set_status)(
            TF_IO_Response* response_context,
            int32_t status_code,
            TF_Status* status
        );
        void (*response__set_header)(
            TF_IO_Response* response_context,
            const TF_TString* name,
            const TF_TString* value,
            TF_Status* status
        );
        void (*response__set_body)(
            TF_IO_Response* response_context,
            const TF_TString* body,
            TF_Status* status
        );
    } TF_IO;

#define TF_IO_STRUCT_SIZE TF_OFFSET_OF_END(TF_IO, response__set_body)

    TF_CAPI_EXPORT void init_io(TF_IO** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_IO_CONTROLLER_H_
