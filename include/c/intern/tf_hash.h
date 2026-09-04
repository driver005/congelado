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

#ifndef TENSORFLOW_C_TF_HASH_H_
#define TENSORFLOW_C_TF_HASH_H_

#include "c/abi/macros.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // Default hashing helpers (std::hash equivalent) for callers of TF_Map /
    // TF_Set that don't need a specialized hash_fn of their own. Not a vtable
    // type — plain functions, the same "global helper" shape as
    // datatype_size in tf_datatype.h.

    // FNV-1a hash of size bytes starting at data. Suitable as a
    // TF_MapHashFn/TF_SetHashFn (see tf_map.h/tf_set.h) for byte-comparable
    // keys (integers, fixed-layout structs, raw buffers).
    TF_CAPI_EXPORT size_t hash_bytes(const void* data, size_t size);

    // Combine an existing hash (seed) with the hash of one more field, in
    // the style of boost::hash_combine. Use to build a hash_fn for a key
    // made of several fields, each hashed with hash_bytes (or itself) and
    // folded together in field order.
    TF_CAPI_EXPORT size_t hash_combine(size_t seed, size_t value);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HASH_H_
