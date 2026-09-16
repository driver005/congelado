#ifndef CONGELADO_C_FILESYSTEM_READ_ONLY_MEMORY_REGION_H_
#define CONGELADO_C_FILESYSTEM_READ_ONLY_MEMORY_REGION_H_

#include "c/macros.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_ReadOnlyMemoryRegion
    {
        void* plugin_data;
    } TF_ReadOnlyMemoryRegion;

    typedef struct TF_ReadOnlyMemoryRegionOps
    {
        size_t struct_size;
        void (*destroy)(TF_ReadOnlyMemoryRegion* region);
        void (*get_name)(void* plugin_context, TF_String* out);

        TF_ReadOnlyMemoryRegion* (*new_read_only_memory_region_from_file)(
            void* plugin_context,
            const TF_String* path,
            TF_Status* status
        );

        const void* (*data)(TF_ReadOnlyMemoryRegion* region);
        uint64_t (*length)(TF_ReadOnlyMemoryRegion* region);
    } TF_ReadOnlyMemoryRegionOps;

#define TF_READ_ONLY_MEMORY_REGION_STRUCT_SIZE TF_OFFSET_OF_END(TF_ReadOnlyMemoryRegionOps, length)

    TF_CAPI_EXPORT void
    create_read_only_memory_region(TF_ReadOnlyMemoryRegionOps** ops, void** plugin_context, TF_Status* status);
    TF_CAPI_EXPORT void destroy_read_only_memory_region(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // CONGELADO_C_FILESYSTEM_READ_ONLY_MEMORY_REGION_H_
