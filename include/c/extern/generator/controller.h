/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

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

#ifndef CONGELADO_C_GENERATOR_CONTROLLER_H_
#define CONGELADO_C_GENERATOR_CONTROLLER_H_

#include "c/abi/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Generator_Controller TF_Generator_Controller;
    typedef struct TF_Generator_Definition TF_Generator_Definition;
    typedef struct TF_Generator_Function TF_Generator_Function;

    TF_CAPI_EXPORT extern TF_Generator_Controller*
    TF_Generator_Create(const char* output_dir, const char* source_dir, TF_Status* status);
    TF_CAPI_EXPORT extern void TF_Generator_Destroy(TF_Generator_Controller* controller);
    // The generator's identity — set by the caller (e.g. "stablehlo") so a registry can
    // hold more than one generator. `name` is copied; `out` is assigned a view into the
    // stored name (caller owns init/destroy of `out`).
    TF_CAPI_EXPORT extern void
    TF_Generator_SetName(TF_Generator_Controller* controller, const TF_String* name);
    TF_CAPI_EXPORT extern void
    TF_Generator_GetName(const TF_Generator_Controller* controller, TF_String* out);
    TF_CAPI_EXPORT extern size_t
    TF_Generator_GetDefinitionCount(const TF_Generator_Controller* controller);
    TF_CAPI_EXPORT extern const TF_Generator_Definition*
    TF_Generator_GetDefinition(const TF_Generator_Controller* controller, size_t index);

    // Renders whatever this generator holds into text. `out` is assigned the rendered text on
    // success (caller owns init/destroy of `out`, same convention as TF_Generator_GetName);
    // returns false on failure (see status), `out` left untouched.
    TF_CAPI_EXPORT extern bool TF_Generator_Build(
        TF_Generator_Controller* controller, TF_String* out, TF_Status* status
    );

    // Generic construction path — mirrors ice::builder::generator::Builder's
    // enter_border_patrol and the ice::builder::generator::Function handle it returns (named
    // after crossing a checkpoint at the C ABI border — deliberately not "scope" or "function"
    // or "block", since those already mean something more specific in most target languages).
    // Carries no generator-specific vocabulary: `kind` and the attr keys/values are plain
    // strings, node results are plain opaque ids, so this crosses the C ABI the same way for
    // any target language a generator emits. A generator that doesn't support construction
    // reports failure (NULL / false).

    // Opens a new named construction unit (`name`), returning an owned handle to build it with
    // via the TF_Generator_Function_* functions below (destroy with
    // TF_Generator_Function_Destroy). NULL if unsupported or on failure (see status).
    TF_CAPI_EXPORT extern TF_Generator_Function* TF_Generator_EnterBorderPatrol(
        TF_Generator_Controller* controller, const char* name, TF_Status* status
    );
    TF_CAPI_EXPORT extern void TF_Generator_Function_Destroy(TF_Generator_Function* function);
    // Adds one input parameter, named `name`, typed by `type_text` (opaque to this interface —
    // the target generator's own textual type syntax). Writes the new parameter's opaque node
    // id into `out_id` and returns true on success; false on failure (see status).
    TF_CAPI_EXPORT extern bool TF_Generator_Function_AddParameter(
        TF_Generator_Function* function,
        const char* name,
        const char* type_text,
        size_t* out_id,
        TF_Status* status
    );
    // Adds one construct of the given `kind`, taking `operand_ids` and `attr_keys`/
    // `attr_values` (parallel arrays, `attr_count` entries), writing `result_count` result ids
    // into caller-allocated `out_result_ids`.
    TF_CAPI_EXPORT extern void TF_Generator_Function_AddNode(
        TF_Generator_Function* function,
        const char* kind,
        const size_t* operand_ids,
        size_t operand_count,
        const char* const* attr_keys,
        const char* const* attr_values,
        size_t attr_count,
        size_t result_count,
        size_t* out_result_ids,
        TF_Status* status
    );
    // Closes this unit, marking `output_ids` as its outputs. Returns false on failure.
    TF_CAPI_EXPORT extern bool TF_Generator_Function_ExitBorderPatrol(
        TF_Generator_Function* function,
        const size_t* output_ids,
        size_t output_count,
        TF_Status* status
    );

#ifdef __cplusplus
}
#endif

#endif // CONGELADO_C_GENERATOR_CONTROLLER_H_
