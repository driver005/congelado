#ifndef TENSORFLOW_C_TF_HASH_H_
#define TENSORFLOW_C_TF_HASH_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

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

        void (*get_name)(TF_Hash* hash, TF_String* out_name);

        // FNV-1a hash of size bytes starting at data.
        void (*hash_bytes)(TF_Hash* hash, const void* data, size_t size, size_t* out_hash, TF_Status* out_status);

        // Combine an existing hash (seed) with the hash of one more field.
        void (*hash_combine)(TF_Hash* hash, size_t seed, size_t value, size_t* out_hash, TF_Status* out_status);

    } TF_HashOps;

#define TF_HASH_STRUCT_SIZE TF_OFFSET_OF_END(TF_HashOps, hash_combine)

    TF_CAPI_EXPORT void create_hash(TF_HashOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_hash(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_HASH_H_
