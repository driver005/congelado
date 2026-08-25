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

#ifndef CONGELADO_C_IO_OPAQUE_TYPES_H_
#define CONGELADO_C_IO_OPAQUE_TYPES_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations only — TP_IO and its New/Delete/Set* inlines live in
// registration.h (full struct definition there); TP_IO_Request in request.h;
// TP_IO_Response in response.h.

struct TP_IO_Request;
struct TP_IO_Response;
struct TP_IO;

#ifdef __cplusplus
}
#endif

#endif  // CONGELADO_C_IO_OPAQUE_TYPES_H_