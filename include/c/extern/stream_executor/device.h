#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_

#include "include/c/macros.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct TF_Device { void* plugin_data; } TF_Device;

    // TF_DeviceOps
    typedef struct TF_DeviceOps {
        size_t struct_size;
        void (*get_numa_node)(TF_Device* device, int32_t* out_numa_node);
        void (*get_memory_bandwidth)(TF_Device* device, int64_t* out_bandwidth);
        void (*get_gflops)(TF_Device* device, double* out_gflops);
        void (*get_hardware_name)(TF_Device* device, TF_String* out_name);
        void (*get_device_vendor)(TF_Device* device, TF_String* out_vendor);
        void (*get_pci_bus_id)(TF_Device* device, TF_String* out_pci_bus_id);
    } TF_DeviceOps;
    #define TF_DEVICE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DeviceOps, get_pci_bus_id)
    
    TF_CAPI_EXPORT void create_device(TF_DeviceOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_device(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif  // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_
