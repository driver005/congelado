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

#ifndef CONGELADO_C_FILESYSTEM_READ_ONLY_MEMORY_REGION_H_
#define CONGELADO_C_FILESYSTEM_READ_ONLY_MEMORY_REGION_H_

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /// Opaque data structure to hold plugin-specific data for a read-only memory
    /// region.
    ///
    /// The wrapper data structure is owned by core TensorFlow. The data pointed to
    /// by the `void*` member is always owned by the plugin.
    ///
    /// Plugins will never receive a `TF_ReadOnlyMemoryRegion*` that is `nullptr`.
    /// Core TensorFlow will never touch the `void*` wrapped by this structure,
    /// except to initialize it as `nullptr`.

    typedef struct TF_ReadOnlyMemoryRegion
    {
        void* plugin_memory_region;
    } TF_ReadOnlyMemoryRegion;

    // --------------------------------------------------------------------------
    // ReadOnlyMemoryRegion callback types

    /// Releases resources associated with `*region`.
    ///
    /// Requires that `*region` is not used in any concurrent or subsequent
    /// operations.
    typedef void (*TP_ReadOnlyMemoryRegion_Cleanup)(TF_ReadOnlyMemoryRegion* region);

    /// Returns a pointer to the memory region.
    typedef const void* (*TP_ReadOnlyMemoryRegion_Data)(const TF_ReadOnlyMemoryRegion* region);

    /// Returns the length of the memory region in bytes.
    typedef uint64_t (*TP_ReadOnlyMemoryRegion_Length)(const TF_ReadOnlyMemoryRegion* region);

    // --------------------------------------------------------------------------
    // ReadOnlyMemoryRegion vtable

    // LINT.IfChange
    typedef struct TF_ReadOnlyMemoryRegionOps
    {
        /// Releases resources associated with `*region`.
        ///
        /// Requires that `*region` is not used in any concurrent or subsequent
        /// operations.
        ///
        /// This operation must be provided. See "REQUIRED OPERATIONS" above.
        TP_ReadOnlyMemoryRegion_Cleanup cleanup;

        /// Returns a pointer to the memory region.
        ///
        /// This operation must be provided. See "REQUIRED OPERATIONS" above.
        TP_ReadOnlyMemoryRegion_Data data;

        /// Returns the length of the memory region in bytes.
        ///
        /// This operation must be provided. See "REQUIRED OPERATIONS" above.
        TP_ReadOnlyMemoryRegion_Length length;
    } TF_ReadOnlyMemoryRegionOps;

    // LINT.ThenChange(:read_only_memory_region_ops_version)

    // Opaque handle alloc/dealloc


    static inline TF_ReadOnlyMemoryRegion* TF_ReadOnlyMemoryRegionNew(void)
    {
        TF_ReadOnlyMemoryRegion* ptr =
            (TF_ReadOnlyMemoryRegion*)malloc(sizeof(TF_ReadOnlyMemoryRegion));
        if (!ptr) {
            return nullptr;
        }
        ptr->plugin_memory_region = nullptr;
        return ptr;
    }

    static inline void TF_ReadOnlyMemoryRegionDelete(TF_ReadOnlyMemoryRegion* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void TF_ReadOnlyMemoryRegion_SetPluginMemoryRegion(
        TF_ReadOnlyMemoryRegion* builder, void* plugin_memory_region
    )
    {
        builder->plugin_memory_region = plugin_memory_region;
    }

    // Vtable alloc/dealloc


    static inline TF_ReadOnlyMemoryRegionOps* TF_ReadOnlyMemoryRegionOpsNew(void)
    {
        TF_ReadOnlyMemoryRegionOps* ptr =
            (TF_ReadOnlyMemoryRegionOps*)malloc(sizeof(TF_ReadOnlyMemoryRegionOps));
        if (!ptr) {
            return nullptr;
        }
        ptr->cleanup = nullptr;
        ptr->data = nullptr;
        ptr->length = nullptr;
        return ptr;
    }

    static inline void TF_ReadOnlyMemoryRegionOpsDelete(TF_ReadOnlyMemoryRegionOps* ptr)
    {
        if (!ptr) {
            return;
        }
        free(ptr);
    }

    static inline void TF_ReadOnlyMemoryRegionOps_SetCleanup(
        TF_ReadOnlyMemoryRegionOps* builder, TP_ReadOnlyMemoryRegion_Cleanup cleanup
    )
    {
        builder->cleanup = cleanup;
    }

    static inline void TF_ReadOnlyMemoryRegionOps_SetData(
        TF_ReadOnlyMemoryRegionOps* builder, TP_ReadOnlyMemoryRegion_Data data
    )
    {
        builder->data = data;
    }

    static inline void TF_ReadOnlyMemoryRegionOps_SetLength(
        TF_ReadOnlyMemoryRegionOps* builder, TP_ReadOnlyMemoryRegion_Length length
    )
    {
        builder->length = length;
    }

#ifdef __cplusplus
} // end extern "C"
#endif // __cplusplus

#endif // CONGELADO_C_FILESYSTEM_READ_ONLY_MEMORY_REGION_H_
