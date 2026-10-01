#ifndef CONGELADO_C_EXTERN_MEMORY_ALLOCATOR_H_
#define CONGELADO_C_EXTERN_MEMORY_ALLOCATOR_H_

#include "include/c/extern/stream_executor/mem_pool.h"
#include "include/c/extern/stream_executor/stream.h"
#include "include/c/extern/stream_executor/types.h"
#include "include/c/intern/buffer.h"
#include "include/c/intern/status.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Allocator
    {
        void* plugin_data;
    } TF_Allocator;

    typedef struct TF_AllocatorStats
    {
        size_t struct_size;
        int64_t num_allocs;
        int64_t bytes_in_use;
        int64_t peak_bytes_in_use;
        int64_t largest_alloc_size;
        int8_t has_bytes_limit;
        int64_t bytes_limit;
        int64_t bytes_reserved;
        int64_t peak_bytes_reserved;
        int8_t has_bytes_reservable_limit;
        int64_t bytes_reservable_limit;
        int64_t largest_free_block_bytes;
    } TF_AllocatorStats;

    typedef enum TF_AllocatorOption
    {
        TF_ALLOCATOR_OPTION_EXPANDABLE_SEGMENTS = 0,
        TF_ALLOCATOR_OPTION_NO_SPLIT = 1,
        TF_ALLOCATOR_OPTION_USE_ON_OOM = 2,
        TF_ALLOCATOR_OPTION_MAX_SPLIT_SIZE = 3,
        TF_ALLOCATOR_OPTION_GARBAGE_COLLECTION_THRESHOLD = 4,
        TF_ALLOCATOR_OPTION_ROUND_UP_POWER2_DIVISIONS = 5,
    } TF_AllocatorOption;

    // TF_AllocatorOps — one allocator per device, created by
    // TF_ExecutorOps::create_allocator_internal. Replaces XPUCachingAllocator, CachingHostAllocator
    // and XPUPluggableAllocator.
    typedef struct TF_AllocatorOps
    {
        size_t struct_size;
        void (*create)(TF_Allocator* out_handle);
        void (*destroy)(TF_Allocator* handle);
        // allocation (stream == NULL means the device's current stream)
        void (*allocate)(
            TF_Allocator* allocator,
            uint64_t size,
            TF_MemorySpace memory_space,
            TF_Stream* stream,
            TF_DeviceMemoryBase* out_memory,
            TF_Status* out_status
        );
        void (*deallocate)(TF_Allocator* allocator, TF_DeviceMemoryBase* memory);
        void (*record_stream)(
            TF_Allocator* allocator,
            const TF_DeviceMemoryBase* memory,
            TF_Stream* stream,
            TF_Status* out_status
        );
        void (*owns_pointer)(TF_Allocator* allocator, const void* pointer, bool* out_owns);
        void (*get_base_allocation)(
            TF_Allocator* allocator,
            const void* pointer,
            void** out_base,
            uint64_t* out_size,
            TF_Status* out_status
        );
        // cache control
        void (*empty_cache)(TF_Allocator* allocator, TF_Status* out_status);
        void (*set_memory_fraction)(
            TF_Allocator* allocator,
            double fraction,
            TF_Status* out_status
        );
        void (*get_memory_fraction)(TF_Allocator* allocator, double* out_fraction);
        void (*set_option)(
            TF_Allocator* allocator,
            TF_AllocatorOption option,
            int64_t value,
            TF_Status* out_status
        );
        void (*get_option)(
            TF_Allocator* allocator,
            TF_AllocatorOption option,
            int64_t* out_value,
            TF_Status* out_status
        );
        // statistics
        void (*get_stats)(TF_Allocator* allocator, TF_AllocatorStats* out_stats, bool* out_success);
        void (*reset_accumulated_stats)(TF_Allocator* allocator);
        void (*reset_peak_stats)(TF_Allocator* allocator);
        void (*get_snapshot)(
            TF_Allocator* allocator,
            const TF_PoolId* pool_filter,
            TF_Buffer* out_snapshot,
            TF_Status* out_status
        );
        // pools (pool_id NULL = allocate a fresh id)
        void (*generate_pool_id)(TF_Allocator* allocator, TF_PoolId* out_pool_id);
        void (*create_mem_pool_internal)(
            TF_Allocator* allocator,
            const TF_PoolId* pool_id,
            bool is_user_created,
            TF_MemPool* out_pool,
            TF_Status* out_status
        );
        void (*destroy_mem_pool_internal)(TF_Allocator* allocator, TF_MemPool* pool);
        // peer access + IPC
        void (*enable_peer_access)(
            TF_Allocator* allocator,
            int peer_device_index,
            TF_Status* out_status
        );
        void (*export_memory)(
            TF_Allocator* allocator,
            const TF_DeviceMemoryBase* memory,
            TF_IpcMemoryHandle* out_handle,
            TF_Status* out_status
        );
        void (*open_memory)(
            TF_Allocator* allocator,
            const TF_IpcMemoryHandle* handle,
            TF_DeviceMemoryBase* out_memory,
            TF_Status* out_status
        );
        void (*close_memory)(
            TF_Allocator* allocator,
            TF_DeviceMemoryBase* memory,
            TF_Status* out_status
        );
    } TF_AllocatorOps;

#define TF_ALLOCATOR_STRUCT_SIZE TF_OFFSET_OF_END(TF_AllocatorOps, close_memory)

    TF_CAPI_EXPORT void
    create_allocator(TF_AllocatorOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_allocator(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_EXTERN_MEMORY_ALLOCATOR_H_
