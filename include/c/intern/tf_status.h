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

#ifndef TENSORFLOW_C_TF_STATUS_H_
#define TENSORFLOW_C_TF_STATUS_H_

#include "c/macros.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Code holds an error code.  The enum values here are identical to
    // corresponding values in error_codes.proto.
    // LINT.IfChange
    typedef enum TF_Code
    {
        TF_OK = 0,
        TF_CANCELLED = 1,
        TF_UNKNOWN = 2,
        TF_INVALID_ARGUMENT = 3,
        TF_DEADLINE_EXCEEDED = 4,
        TF_NOT_FOUND = 5,
        TF_ALREADY_EXISTS = 6,
        TF_PERMISSION_DENIED = 7,
        TF_UNAUTHENTICATED = 16,
        TF_RESOURCE_EXHAUSTED = 8,
        TF_FAILED_PRECONDITION = 9,
        TF_ABORTED = 10,
        TF_OUT_OF_RANGE = 11,
        TF_UNIMPLEMENTED = 12,
        TF_INTERNAL = 13,
        TF_UNAVAILABLE = 14,
        TF_DATA_LOSS = 15
    } TF_Code;

    // LINT.ThenChange(//tensorflow/python/py_exception_registry_wrapper.cc)

    // Callback type for iterating over payloads.
    typedef void (*TF_PayloadVisitor)(
        const char* key,
        size_t key_len,
        const char* value,
        size_t value_len,
        void* capture
    );

    // --------------------------------------------------------------------------

    // Opaque status object. Owned and laid out entirely by whichever backend's
    // init_status() supplied the TF_Status ops below — callers never look inside it,
    // only ever hold/pass a pointer.
    typedef struct TF_Status_Handle TF_Status_Handle;

    // Ops vtable for TF_Status_Handle — matches the intern/extern convention (struct_size
    // first, every slot takes plugin_context first) instead of free functions, so this type
    // registers with cc_abi_gen exactly like TF_Buffer/TF_Shape/TF_String.
    typedef struct TF_Status
    {
        size_t struct_size;

        // Return a new status object.
        TF_Status_Handle* (*new_status)(void* plugin_context);

        // Delete a previously created status object.
        void (*delete_status)(TF_Status_Handle* s);

        // Record <code, msg> in *s.  Any previous information is lost.
        // A common use is to clear a status: set_status(s, TF_OK, "");
        void (*set_status)(TF_Status_Handle* s, TF_Code code, const char* msg);

        // Record <key, value> as a payload in *s. The previous payload having the
        // same key (if any) is overwritten. Payload will not be added if the Status
        // is OK.
        void (*set_payload)(TF_Status_Handle* s, const char* key, const char* value);

        // Iterates over the stored payloads and calls the `visitor(key, value)`
        // callable for each one. `key` and `value` is only usable during the callback.
        // `capture` will be passed to the callback without modification.
        void (*for_each_payload)(
            const TF_Status_Handle* s,
            TF_PayloadVisitor visitor,
            void* capture
        );

        // Convert from an I/O error code (e.g., errno) to a TF_Status value.
        // Any previous information is lost. Prefer to use this instead of set_status
        // when the error comes from I/O operations.
        void (*set_status_from_io_error)(TF_Status_Handle* s, int error_code, const char* context);

        // Return the code record in *s.
        TF_Code (*get_code)(const TF_Status_Handle* s);

        // Return a pointer to the (null-terminated) error message in *s.  The
        // return value points to memory that is only usable until the next
        // mutation to *s.  Always returns an empty string if get_code(s) is
        // TF_OK.
        const char* (*message)(const TF_Status_Handle* s);
    } TF_Status;

#define TF_STATUS_STRUCT_SIZE TF_OFFSET_OF_END(TF_Status, message)

    // Declared-only, like init_buffer/init_shape/init_string: no default implementation
    // lives under include/c/, graceful null-ops degradation expected. Same signature shape
    // as every other init_* — status may be null on this call (nothing yet exists to have
    // produced a non-null one), and callees must already tolerate that per the null-ops
    // degradation contract.
    TF_CAPI_EXPORT void init_status(TF_Status** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_STATUS_H_
