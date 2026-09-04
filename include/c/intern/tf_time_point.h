/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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

#ifndef TENSORFLOW_C_TF_TIME_POINT_H_
#define TENSORFLOW_C_TF_TIME_POINT_H_

#include "c/abi/macros.h"
#include "c/intern/tf_duration.h"
#include "c/intern/tf_status.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_TimePoint — plugin vtable for a point in time expressed as a
    // duration since an implementation-defined epoch (std::chrono::time_point
    // equivalent).
    //
    // TF_TimePoint_Handle is an opaque pointer to a plugin-owned time_point
    // object.
    typedef struct TF_TimePoint_Handle TF_TimePoint_Handle;

    // Plugin-facing vtable registered via init_time_point.
    typedef struct TF_TimePoint
    {
        size_t struct_size;

        // Allocate a new time_point ticks * (ratio_num / ratio_den) seconds
        // since the epoch. Must be freed with destroy.
        TF_TimePoint_Handle* (*new_time_point)(
            void* plugin_context,
            int64_t ticks,
            int64_t ratio_num,
            int64_t ratio_den
        );

        // A duration handle (obtained via TF_Duration, see tf_duration.h)
        // representing the elapsed time since the epoch. Owned by the
        // time_point; do not destroy independently.
        TF_Duration_Handle* (*get_duration_since_epoch)(
            void* plugin_context,
            const TF_TimePoint_Handle* time_point
        );

        // Free a handle returned by new_time_point.
        void (*destroy)(void* plugin_context, TF_TimePoint_Handle* time_point);

    } TF_TimePoint;

#define TF_TIME_POINT_STRUCT_SIZE TF_OFFSET_OF_END(TF_TimePoint, destroy)

    TF_CAPI_EXPORT void
    init_time_point(TF_TimePoint** ops, void** plugin_context, TF_Status* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_TIME_POINT_H_
