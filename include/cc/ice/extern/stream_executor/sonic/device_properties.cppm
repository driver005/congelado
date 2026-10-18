// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/device.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/device.h"

export module cc_ice_extern_stream_executor_sonic:device_properties;

import std;

export namespace ice::sonic {

class DeviceProperties
{
public:
    DeviceProperties() noexcept
    {
        m_value.struct_size = sizeof(::TF_DeviceProperties);
    }

    explicit DeviceProperties(const ::TF_DeviceProperties& value) noexcept :
        m_value{value}
    {
    }

    std::span<char, 256> getName() noexcept
    {
        return m_value.name;
    }

    std::span<const char, 256> getName() const noexcept
    {
        return m_value.name;
    }

    std::span<char, 128> getVendor() noexcept
    {
        return m_value.vendor;
    }

    std::span<const char, 128> getVendor() const noexcept
    {
        return m_value.vendor;
    }

    std::span<char, 64> getDriverVersion() noexcept
    {
        return m_value.driver_version;
    }

    std::span<const char, 64> getDriverVersion() const noexcept
    {
        return m_value.driver_version;
    }

    std::span<char, 64> getVersion() noexcept
    {
        return m_value.version;
    }

    std::span<const char, 64> getVersion() const noexcept
    {
        return m_value.version;
    }

    std::int32_t getDeviceType() const noexcept
    {
        return m_value.device_type;
    }

    void setDeviceType(std::int32_t value) noexcept
    {
        m_value.device_type = value;
    }

    _Bool getIsAvailable() const noexcept
    {
        return m_value.is_available;
    }

    void setIsAvailable(_Bool value) noexcept
    {
        m_value.is_available = value;
    }

    std::span<std::uint8_t, 16> getUuid() noexcept
    {
        return m_value.uuid;
    }

    std::span<const std::uint8_t, 16> getUuid() const noexcept
    {
        return m_value.uuid;
    }

    std::int32_t getDeviceId() const noexcept
    {
        return m_value.device_id;
    }

    void setDeviceId(std::int32_t value) noexcept
    {
        m_value.device_id = value;
    }

    std::int32_t getArchitecture() const noexcept
    {
        return m_value.architecture;
    }

    void setArchitecture(std::int32_t value) noexcept
    {
        m_value.architecture = value;
    }

    std::uint64_t getGlobalMemSize() const noexcept
    {
        return m_value.global_mem_size;
    }

    void setGlobalMemSize(std::uint64_t value) noexcept
    {
        m_value.global_mem_size = value;
    }

    std::uint64_t getLocalMemSize() const noexcept
    {
        return m_value.local_mem_size;
    }

    void setLocalMemSize(std::uint64_t value) noexcept
    {
        m_value.local_mem_size = value;
    }

    std::uint64_t getMaxMemAllocSize() const noexcept
    {
        return m_value.max_mem_alloc_size;
    }

    void setMaxMemAllocSize(std::uint64_t value) noexcept
    {
        m_value.max_mem_alloc_size = value;
    }

    std::uint64_t getGlobalMemCacheSize() const noexcept
    {
        return m_value.global_mem_cache_size;
    }

    void setGlobalMemCacheSize(std::uint64_t value) noexcept
    {
        m_value.global_mem_cache_size = value;
    }

    std::uint32_t getGlobalMemCacheLineSize() const noexcept
    {
        return m_value.global_mem_cache_line_size;
    }

    void setGlobalMemCacheLineSize(std::uint32_t value) noexcept
    {
        m_value.global_mem_cache_line_size = value;
    }

    std::uint32_t getGlobalMemCacheType() const noexcept
    {
        return m_value.global_mem_cache_type;
    }

    void setGlobalMemCacheType(std::uint32_t value) noexcept
    {
        m_value.global_mem_cache_type = value;
    }

    std::uint32_t getLocalMemType() const noexcept
    {
        return m_value.local_mem_type;
    }

    void setLocalMemType(std::uint32_t value) noexcept
    {
        m_value.local_mem_type = value;
    }

    std::uint32_t getMemBaseAddrAlign() const noexcept
    {
        return m_value.mem_base_addr_align;
    }

    void setMemBaseAddrAlign(std::uint32_t value) noexcept
    {
        m_value.mem_base_addr_align = value;
    }

    std::uint32_t getMemoryClockRate() const noexcept
    {
        return m_value.memory_clock_rate;
    }

    void setMemoryClockRate(std::uint32_t value) noexcept
    {
        m_value.memory_clock_rate = value;
    }

    std::uint32_t getMemoryBusWidth() const noexcept
    {
        return m_value.memory_bus_width;
    }

    void setMemoryBusWidth(std::uint32_t value) noexcept
    {
        m_value.memory_bus_width = value;
    }

    std::uint32_t getMaxComputeUnits() const noexcept
    {
        return m_value.max_compute_units;
    }

    void setMaxComputeUnits(std::uint32_t value) noexcept
    {
        m_value.max_compute_units = value;
    }

    std::uint32_t getMaxWorkItemDimensions() const noexcept
    {
        return m_value.max_work_item_dimensions;
    }

    void setMaxWorkItemDimensions(std::uint32_t value) noexcept
    {
        m_value.max_work_item_dimensions = value;
    }

    std::uint32_t getMaxWorkGroupSize() const noexcept
    {
        return m_value.max_work_group_size;
    }

    void setMaxWorkGroupSize(std::uint32_t value) noexcept
    {
        m_value.max_work_group_size = value;
    }

    std::uint32_t getMaxNumSubGroups() const noexcept
    {
        return m_value.max_num_sub_groups;
    }

    void setMaxNumSubGroups(std::uint32_t value) noexcept
    {
        m_value.max_num_sub_groups = value;
    }

    std::span<std::uint32_t, 16> getSubGroupSizes() noexcept
    {
        return m_value.sub_group_sizes;
    }

    std::span<const std::uint32_t, 16> getSubGroupSizes() const noexcept
    {
        return m_value.sub_group_sizes;
    }

    std::uint32_t getNumSubGroupSizes() const noexcept
    {
        return m_value.num_sub_group_sizes;
    }

    void setNumSubGroupSizes(std::uint32_t value) noexcept
    {
        m_value.num_sub_group_sizes = value;
    }

    std::uint32_t getMaxClockFrequency() const noexcept
    {
        return m_value.max_clock_frequency;
    }

    void setMaxClockFrequency(std::uint32_t value) noexcept
    {
        m_value.max_clock_frequency = value;
    }

    std::uint32_t getAddressBits() const noexcept
    {
        return m_value.address_bits;
    }

    void setAddressBits(std::uint32_t value) noexcept
    {
        m_value.address_bits = value;
    }

    std::uint32_t getMaxParameterSize() const noexcept
    {
        return m_value.max_parameter_size;
    }

    void setMaxParameterSize(std::uint32_t value) noexcept
    {
        m_value.max_parameter_size = value;
    }

    std::uint32_t getPartitionMaxSubDevices() const noexcept
    {
        return m_value.partition_max_sub_devices;
    }

    void setPartitionMaxSubDevices(std::uint32_t value) noexcept
    {
        m_value.partition_max_sub_devices = value;
    }

    std::uint64_t getProfilingTimerResolution() const noexcept
    {
        return m_value.profiling_timer_resolution;
    }

    void setProfilingTimerResolution(std::uint64_t value) noexcept
    {
        m_value.profiling_timer_resolution = value;
    }

    std::uint32_t getHalfFpConfig() const noexcept
    {
        return m_value.half_fp_config;
    }

    void setHalfFpConfig(std::uint32_t value) noexcept
    {
        m_value.half_fp_config = value;
    }

    std::uint32_t getSingleFpConfig() const noexcept
    {
        return m_value.single_fp_config;
    }

    void setSingleFpConfig(std::uint32_t value) noexcept
    {
        m_value.single_fp_config = value;
    }

    std::uint32_t getDoubleFpConfig() const noexcept
    {
        return m_value.double_fp_config;
    }

    void setDoubleFpConfig(std::uint32_t value) noexcept
    {
        m_value.double_fp_config = value;
    }

    std::uint32_t getPreferredVectorWidthChar() const noexcept
    {
        return m_value.preferred_vector_width_char;
    }

    void setPreferredVectorWidthChar(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_char = value;
    }

    std::uint32_t getPreferredVectorWidthShort() const noexcept
    {
        return m_value.preferred_vector_width_short;
    }

    void setPreferredVectorWidthShort(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_short = value;
    }

    std::uint32_t getPreferredVectorWidthInt() const noexcept
    {
        return m_value.preferred_vector_width_int;
    }

    void setPreferredVectorWidthInt(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_int = value;
    }

    std::uint32_t getPreferredVectorWidthLong() const noexcept
    {
        return m_value.preferred_vector_width_long;
    }

    void setPreferredVectorWidthLong(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_long = value;
    }

    std::uint32_t getPreferredVectorWidthFloat() const noexcept
    {
        return m_value.preferred_vector_width_float;
    }

    void setPreferredVectorWidthFloat(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_float = value;
    }

    std::uint32_t getPreferredVectorWidthDouble() const noexcept
    {
        return m_value.preferred_vector_width_double;
    }

    void setPreferredVectorWidthDouble(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_double = value;
    }

    std::uint32_t getPreferredVectorWidthHalf() const noexcept
    {
        return m_value.preferred_vector_width_half;
    }

    void setPreferredVectorWidthHalf(std::uint32_t value) noexcept
    {
        m_value.preferred_vector_width_half = value;
    }

    std::uint32_t getNativeVectorWidthChar() const noexcept
    {
        return m_value.native_vector_width_char;
    }

    void setNativeVectorWidthChar(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_char = value;
    }

    std::uint32_t getNativeVectorWidthShort() const noexcept
    {
        return m_value.native_vector_width_short;
    }

    void setNativeVectorWidthShort(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_short = value;
    }

    std::uint32_t getNativeVectorWidthInt() const noexcept
    {
        return m_value.native_vector_width_int;
    }

    void setNativeVectorWidthInt(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_int = value;
    }

    std::uint32_t getNativeVectorWidthLong() const noexcept
    {
        return m_value.native_vector_width_long;
    }

    void setNativeVectorWidthLong(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_long = value;
    }

    std::uint32_t getNativeVectorWidthFloat() const noexcept
    {
        return m_value.native_vector_width_float;
    }

    void setNativeVectorWidthFloat(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_float = value;
    }

    std::uint32_t getNativeVectorWidthDouble() const noexcept
    {
        return m_value.native_vector_width_double;
    }

    void setNativeVectorWidthDouble(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_double = value;
    }

    std::uint32_t getNativeVectorWidthHalf() const noexcept
    {
        return m_value.native_vector_width_half;
    }

    void setNativeVectorWidthHalf(std::uint32_t value) noexcept
    {
        m_value.native_vector_width_half = value;
    }

    std::uint32_t getGpuEuCount() const noexcept
    {
        return m_value.gpu_eu_count;
    }

    void setGpuEuCount(std::uint32_t value) noexcept
    {
        m_value.gpu_eu_count = value;
    }

    std::uint32_t getGpuEuCountPerSubslice() const noexcept
    {
        return m_value.gpu_eu_count_per_subslice;
    }

    void setGpuEuCountPerSubslice(std::uint32_t value) noexcept
    {
        m_value.gpu_eu_count_per_subslice = value;
    }

    std::uint32_t getGpuEuSimdWidth() const noexcept
    {
        return m_value.gpu_eu_simd_width;
    }

    void setGpuEuSimdWidth(std::uint32_t value) noexcept
    {
        m_value.gpu_eu_simd_width = value;
    }

    std::uint32_t getGpuHwThreadsPerEu() const noexcept
    {
        return m_value.gpu_hw_threads_per_eu;
    }

    void setGpuHwThreadsPerEu(std::uint32_t value) noexcept
    {
        m_value.gpu_hw_threads_per_eu = value;
    }

    std::uint32_t getXeStackCount() const noexcept
    {
        return m_value.xe_stack_count;
    }

    void setXeStackCount(std::uint32_t value) noexcept
    {
        m_value.xe_stack_count = value;
    }

    std::uint32_t getXeRegionsPerStack() const noexcept
    {
        return m_value.xe_regions_per_stack;
    }

    void setXeRegionsPerStack(std::uint32_t value) noexcept
    {
        m_value.xe_regions_per_stack = value;
    }

    std::uint32_t getXeClustersPerRegion() const noexcept
    {
        return m_value.xe_clusters_per_region;
    }

    void setXeClustersPerRegion(std::uint32_t value) noexcept
    {
        m_value.xe_clusters_per_region = value;
    }

    std::uint32_t getXeCoresPerCluster() const noexcept
    {
        return m_value.xe_cores_per_cluster;
    }

    void setXeCoresPerCluster(std::uint32_t value) noexcept
    {
        m_value.xe_cores_per_cluster = value;
    }

    std::uint32_t getEusPerXeCore() const noexcept
    {
        return m_value.eus_per_xe_core;
    }

    void setEusPerXeCore(std::uint32_t value) noexcept
    {
        m_value.eus_per_xe_core = value;
    }

    std::uint32_t getMaxLanesPerHwThread() const noexcept
    {
        return m_value.max_lanes_per_hw_thread;
    }

    void setMaxLanesPerHwThread(std::uint32_t value) noexcept
    {
        m_value.max_lanes_per_hw_thread = value;
    }

    _Bool getHasFp16() const noexcept
    {
        return m_value.has_fp16;
    }

    void setHasFp16(_Bool value) noexcept
    {
        m_value.has_fp16 = value;
    }

    _Bool getHasFp64() const noexcept
    {
        return m_value.has_fp64;
    }

    void setHasFp64(_Bool value) noexcept
    {
        m_value.has_fp64 = value;
    }

    _Bool getHasAtomic64() const noexcept
    {
        return m_value.has_atomic64;
    }

    void setHasAtomic64(_Bool value) noexcept
    {
        m_value.has_atomic64 = value;
    }

    _Bool getHasBfloat16Conversions() const noexcept
    {
        return m_value.has_bfloat16_conversions;
    }

    void setHasBfloat16Conversions(_Bool value) noexcept
    {
        m_value.has_bfloat16_conversions = value;
    }

    _Bool getHasSubgroupMatrixMultiplyAccumulate() const noexcept
    {
        return m_value.has_subgroup_matrix_multiply_accumulate;
    }

    void setHasSubgroupMatrixMultiplyAccumulate(_Bool value) noexcept
    {
        m_value.has_subgroup_matrix_multiply_accumulate = value;
    }

    _Bool getHasSubgroupMatrixMultiplyAccumulateTensorFloat32() const noexcept
    {
        return m_value.has_subgroup_matrix_multiply_accumulate_tensor_float32;
    }

    void setHasSubgroupMatrixMultiplyAccumulateTensorFloat32(_Bool value) noexcept
    {
        m_value.has_subgroup_matrix_multiply_accumulate_tensor_float32 = value;
    }

    _Bool getHasSubgroup2dBlockIo() const noexcept
    {
        return m_value.has_subgroup_2d_block_io;
    }

    void setHasSubgroup2dBlockIo(_Bool value) noexcept
    {
        m_value.has_subgroup_2d_block_io = value;
    }

    ::TF_DeviceProperties* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_DeviceProperties* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_DeviceProperties m_value{};
};

} // namespace ice::sonic
