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

    typedef struct TF_Hash
    {
        void* plugin_data;
    } TF_Hash;

    typedef struct TF_HashOps
    {
        size_t struct_size;

        void (*get_name)(TF_Hash* hash, TF_String* out);

        // FNV-1a hash of size bytes starting at data.
        size_t (*hash_bytes)(TF_Hash* hash, const void* data, size_t size);

        // Combine an existing hash (seed) with the hash of one more field.
        size_t (*hash_combine)(TF_Hash* hash, size_t seed, size_t value);

    } TF_HashOps;

#define TF_HASH_STRUCT_SIZE TF_OFFSET_OF_END(TF_HashOps, hash_combine)

    TF_CAPI_EXPORT void create_hash(TF_HashOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_hash(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HASH_H_
