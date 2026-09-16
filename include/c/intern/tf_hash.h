#ifndef TENSORFLOW_C_TF_HASH_H_
#define TENSORFLOW_C_TF_HASH_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // --------------------------------------------------------------------------
    // TF_Hash — plugin vtable for default hashing helpers (std::hash equivalent) for
    // callers of TF_Map / TF_Set that don't need a specialized hash_fn of their own.
    typedef struct TF_Hash
    {
        size_t struct_size;

        // Return the backend's name (e.g. "hash") into *out.
        void (*get_name)(void* plugin_context, TF_String* out);

        // FNV-1a hash of size bytes starting at data. Suitable as a
        // TF_MapHashFn/TF_SetHashFn (see tf_map.h/tf_set.h) for byte-comparable
        // keys (integers, fixed-layout structs, raw buffers).
        size_t (*hash_bytes)(void* plugin_context, const void* data, size_t size);

        // Combine an existing hash (seed) with the hash of one more field, in
        // the style of boost::hash_combine. Use to build a hash_fn for a key
        // made of several fields, each hashed with hash_bytes (or itself) and
        // folded together in field order.
        size_t (*hash_combine)(void* plugin_context, size_t seed, size_t value);

    } TF_Hash;

#define TF_HASH_STRUCT_SIZE TF_OFFSET_OF_END(TF_Hash, hash_combine)

    TF_CAPI_EXPORT void init_hash(TF_Hash** ops, void** plugin_context, TF_Status_Handle* status);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HASH_H_
