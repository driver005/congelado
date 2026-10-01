#ifndef TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_
#define TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_

#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/macros.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct TF_Device
    {
        void* plugin_data;
    } TF_Device;

    // POD mirror of c10/XPUDeviceProp.h. Append-only, versioned by struct_size.
    typedef struct TF_DeviceProperties
    {
        size_t struct_size;
        char name[256];
        char vendor[128];
        char driver_version[64];
        char version[64];
        int32_t device_type;
        bool is_available;
        uint8_t uuid[16];
        int32_t device_id;
        int32_t architecture;
        uint64_t global_mem_size;
        uint64_t local_mem_size;
        uint64_t max_mem_alloc_size;
        uint64_t global_mem_cache_size;
        uint32_t global_mem_cache_line_size;
        uint32_t global_mem_cache_type;
        uint32_t local_mem_type;
        uint32_t mem_base_addr_align;
        uint32_t memory_clock_rate;
        uint32_t memory_bus_width;
        uint32_t max_compute_units;
        uint32_t max_work_item_dimensions;
        uint32_t max_work_group_size;
        uint32_t max_num_sub_groups;
        uint32_t sub_group_sizes[16];
        uint32_t num_sub_group_sizes;
        uint32_t max_clock_frequency;
        uint32_t address_bits;
        uint32_t max_parameter_size;
        uint32_t partition_max_sub_devices;
        uint64_t profiling_timer_resolution;
        uint32_t half_fp_config;
        uint32_t single_fp_config;
        uint32_t double_fp_config;
        uint32_t preferred_vector_width_char;
        uint32_t preferred_vector_width_short;
        uint32_t preferred_vector_width_int;
        uint32_t preferred_vector_width_long;
        uint32_t preferred_vector_width_float;
        uint32_t preferred_vector_width_double;
        uint32_t preferred_vector_width_half;
        uint32_t native_vector_width_char;
        uint32_t native_vector_width_short;
        uint32_t native_vector_width_int;
        uint32_t native_vector_width_long;
        uint32_t native_vector_width_float;
        uint32_t native_vector_width_double;
        uint32_t native_vector_width_half;
        uint32_t gpu_eu_count;
        uint32_t gpu_eu_count_per_subslice;
        uint32_t gpu_eu_simd_width;
        uint32_t gpu_hw_threads_per_eu;
        uint32_t xe_stack_count;
        uint32_t xe_regions_per_stack;
        uint32_t xe_clusters_per_region;
        uint32_t xe_cores_per_cluster;
        uint32_t eus_per_xe_core;
        uint32_t max_lanes_per_hw_thread;
        bool has_fp16;
        bool has_fp64;
        bool has_atomic64;
        bool has_bfloat16_conversions;
        bool has_subgroup_matrix_multiply_accumulate;
        bool has_subgroup_matrix_multiply_accumulate_tensor_float32;
        bool has_subgroup_2d_block_io;
    } TF_DeviceProperties;

    // TF_DeviceOps
    typedef struct TF_DeviceOps
    {
        size_t struct_size;
        void (*create)(TF_Device* out_handle);
        void (*destroy)(TF_Device* handle);
        void (*get_numa_node)(TF_Device* device, int32_t* out_numa_node);
        void (*get_memory_bandwidth)(TF_Device* device, int64_t* out_bandwidth);
        void (*get_gflops)(TF_Device* device, double* out_gflops);
        void (*get_hardware_name)(TF_Device* device, TF_String* out_name);
        void (*get_device_vendor)(TF_Device* device, TF_String* out_vendor);
        void (*get_pci_bus_id)(TF_Device* device, TF_String* out_pci_bus_id);
        void (*get_device_properties)(
            TF_Device* device,
            TF_DeviceProperties* out_properties,
            TF_Status* out_status
        );
        void (*get_native_handle)(TF_Device* device, void** out_handle);
    } TF_DeviceOps;

#define TF_DEVICE_STRUCT_SIZE TF_OFFSET_OF_END(TF_DeviceOps, get_native_handle)

    TF_CAPI_EXPORT void
    create_device(TF_DeviceOps** ops, void** plugin_context, TF_Status* out_status);
    TF_CAPI_EXPORT void destroy_device(void* plugin_context);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif // TENSORFLOW_C_EXTERN_STREAM_EXECUTOR_DEVICE_H_
