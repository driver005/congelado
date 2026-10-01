#ifndef CONGELADO_C_EXTERN_MEMORY_MEM_POOL_H_
#define CONGELADO_C_EXTERN_MEMORY_MEM_POOL_H_

#include "include/c/extern/stream_executor/stream.h"
#include "include/c/intern/status.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_PoolId
    {
        int64_t first;
        int64_t second;
    } TF_PoolId;

    typedef struct TF_MemPool
    {
        void* plugin_data;
    } TF_MemPool;

    // Decides whether allocations made on stream are routed to the pool.
    typedef void (*TF_StreamFilterFn)(void* user_data, TF_Stream* stream, bool* out_match);

    // TF_MemPoolOps — created by TF_AllocatorOps::create_mem_pool_internal. Replaces
    // at::xpu::MemPool and the allocator pool APIs.
    typedef struct TF_MemPoolOps
    {
        size_t struct_size;
        void (*create)(TF_MemPool* out_handle);
        void (*destroy)(TF_MemPool* handle);
        void (*get_id)(TF_MemPool* pool, TF_PoolId* out_pool_id);
        void (*use_count)(TF_MemPool* pool, int* out_count);
        void (*begin_allocate_to_pool)(
            TF_MemPool* pool,
            TF_StreamFilterFn stream_filter,
            void* filter_data,
            TF_Status* out_status
        );
        void (*end_allocate_to_pool)(TF_MemPool* pool, TF_Status* out_status);
        void (*release)(TF_MemPool* pool, TF_Status* out_status);
        void (*set_use_on_oom)(TF_MemPool* pool, bool use_on_oom, TF_Status* out_status);
    } TF_MemPoolOps;

#define TF_MEM_POOL_STRUCT_SIZE TF_OFFSET_OF_END(TF_MemPoolOps, set_use_on_oom)

    TF_CAPI_EXPORT void
    create_mem_pool(TF_MemPoolOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_mem_pool(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_EXTERN_MEMORY_MEM_POOL_H_
