#ifndef CONGELADO_C_EXTERN_MEMORY_TYPES_H_
#define CONGELADO_C_EXTERN_MEMORY_TYPES_H_

#include "include/c/macros.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Shared value types for the memory domain. Types only, no vtable.

    typedef enum TF_MemorySpace {
        TF_MEMORY_SPACE_DEVICE = 0,
        TF_MEMORY_SPACE_HOST_PINNED = 1,
        TF_MEMORY_SPACE_UNIFIED = 2,
    } TF_MemorySpace;

    typedef struct TF_DeviceMemoryBase {
        size_t struct_size;
        void* ext;
        void* opaque;
        uint64_t size;
        uint64_t payload;
    } TF_DeviceMemoryBase;

    // Opaque cross-process handle for device memory. data_size bytes of data are valid.
    typedef struct TF_IpcMemoryHandle {
        size_t struct_size;
        uint8_t data[128];
        uint64_t data_size;
        uint64_t allocation_size;
    } TF_IpcMemoryHandle;

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // CONGELADO_C_EXTERN_MEMORY_TYPES_H_
