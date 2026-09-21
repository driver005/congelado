#ifndef CONGELADO_C_EXTERN_MEMORY_MEMORY_H_
#define CONGELADO_C_EXTERN_MEMORY_MEMORY_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include "include/c/extern/memory/types.h"
#include "include/c/extern/memory/mem_pool.h"
#include "include/c/extern/memory/allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Memory {
        void* plugin_data;
        void* allocator_context;
        void* mem_pool_context;
        const TF_AllocatorOps* allocator_ops;
        const TF_MemPoolOps* mem_pool_ops;
    } TF_Memory;

    typedef struct TF_MemoryOps {
        size_t struct_size;
        void (*destroy)(TF_Memory* memory);
        void (*get_name)(TF_Memory* memory, TF_String* out_name);
    } TF_MemoryOps;

    #define TF_MEMORY_STRUCT_SIZE TF_OFFSET_OF_END(TF_MemoryOps, get_name)

    TF_CAPI_EXPORT void create_memory(TF_MemoryOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_memory(void* plugin_context);

    // Each vtable gets its own plugin context slot so create_* calls do not overwrite one another.
    static inline void init_memory(TF_MemoryOps** ops, TF_Memory* memory, TF_Status* out_status) {
        create_memory(ops, &memory->plugin_data, out_status);

        TF_AllocatorOps* allocator_ops = NULL;
        create_allocator(&allocator_ops, &memory->allocator_context, out_status);
        memory->allocator_ops = allocator_ops;

        TF_MemPoolOps* mem_pool_ops = NULL;
        create_mem_pool(&mem_pool_ops, &memory->mem_pool_context, out_status);
        memory->mem_pool_ops = mem_pool_ops;
    }

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // CONGELADO_C_EXTERN_MEMORY_MEMORY_H_
