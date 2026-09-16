#ifndef TENSORFLOW_C_TF_MAP_H_
#define TENSORFLOW_C_TF_MAP_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef size_t (*TF_MapHashFn)(const void* key, size_t key_size);
    typedef int (*TF_MapCompareFn)(const void* lhs, const void* rhs, size_t key_size);
    typedef void (*TF_MapVisitor)(void* capture, const void* key, const void* value);

    typedef struct TF_Map
    {
        void* plugin_data;
    } TF_Map;

    typedef struct TF_MapOps
    {
        size_t struct_size;

        int (*insert)(TF_Map* map, const void* key, const void* value);
        const void* (*find)(const TF_Map* map, const void* key);
        int (*erase)(TF_Map* map, const void* key);
        int (*contains)(const TF_Map* map, const void* key);
        size_t (*size)(const TF_Map* map);
        void (*for_each)(const TF_Map* map, TF_MapVisitor visitor, void* capture);
        void (*destroy)(TF_Map* map);

    } TF_MapOps;

#define TF_MAP_STRUCT_SIZE TF_OFFSET_OF_END(TF_MapOps, destroy)

    TF_CAPI_EXPORT void create_map(TF_MapOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_map(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_TF_MAP_H_
