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
#ifndef CONGELADO_C_EVENTS_H_
#define CONGELADO_C_EVENTS_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef void (*TP_Events_PublishFn)(
        void* user_data, const TF_TString* event_name, const TF_TString* payload_json
    );

    // Forward declaration — TP_Events is fully defined below; needed here for
    // TF_EventsRegistrationParams_DestroyEvents which references TP_Events*.
    typedef struct TP_Events TP_Events;

    typedef void (*TF_EventsRegistrationParams_DestroyEvents)(TP_Events* events);

    typedef struct TP_Events
    {
        size_t struct_size;
        void* ext;
        TF_TString name;
        TP_Events_PublishFn publish_cb;
#define TP_EVENTS_STRUCT_SIZE TF_OFFSET_OF_END(TP_Events, publish_cb)
    } TP_Events;

    typedef struct TF_EventsRegistrationParams
    {
        size_t struct_size;
        void* ext;
        int32_t major_version;
        int32_t minor_version;
        int32_t patch_version;
        TP_Events* events;
        TF_EventsRegistrationParams_DestroyEvents destroy_events;
#define TF_EVENTS_REGISTRATION_PARAMS_STRUCT_SIZE                                                  \
    TF_OFFSET_OF_END(TF_EventsRegistrationParams, destroy_events)
    } TF_EventsRegistrationParams;

#define EV_MAJOR 0
#define EV_MINOR 0
#define EV_PATCH 1

    void TF_InitEvents(TF_EventsRegistrationParams* params, TF_Status* status);

    static inline TP_Events* TP_EventsNew(void)
    {
        TP_Events* ptr = (TP_Events*)malloc(TP_EVENTS_STRUCT_SIZE);
        if (!ptr) {
            return nullptr;
        }
        ptr->struct_size = TP_EVENTS_STRUCT_SIZE;
        ptr->ext = nullptr;
        TF_StringInit(&ptr->name);
        ptr->publish_cb = nullptr;
        return ptr;
    }

    static inline void TP_EventsDelete(TP_Events* ptr)
    {
        if (!ptr) {
            return;
        }
        TF_StringDealloc(&ptr->name);
        free(ptr);
    }

    static inline void TP_Events_SetName(TP_Events* builder, TF_TString name)
    {
        builder->name = name;
    }

    static inline void
    TP_Events_SetPublishCallback(TP_Events* builder, TP_Events_PublishFn publish_cb)
    {
        builder->publish_cb = publish_cb;
    }


#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_EVENTS_H_
