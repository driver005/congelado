module;

#include "include/c/extern/stream_executor/device.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:device;

import std;
import cc_ice_extern_stream_executor_sonic;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclDevice : public ice::builder::Device
{
public:
    explicit SyclDevice(const SyclOpsTable& ops) noexcept :
        ice::builder::Device{ops.getStatusOps(), ops.getStringOps()},
        m_status{ops}
    {
    }

    ~SyclDevice() override = default;
    SyclDevice(const SyclDevice&) = delete;
    SyclDevice& operator=(const SyclDevice&) = delete;
    SyclDevice(SyclDevice&&) = delete;
    SyclDevice& operator=(SyclDevice&&) = delete;

    void bind(const sycl::device& device, int device_index)
    {
        m_device = device;
        m_device_index = device_index;
        m_global_index = resolve_global_index(m_device);
        fill_properties();
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_numa_node(std::int32_t& out_numa_node) noexcept override
    {
        out_numa_node = -1;
    }

    void get_memory_bandwidth(std::int64_t& out_bandwidth) noexcept override
    {
        const auto bus_bits = static_cast<int64_t>(m_properties.getMemoryBusWidth());
        const auto clock_khz = static_cast<int64_t>(m_properties.getMemoryClockRate());
        out_bandwidth = bus_bits / 8 * clock_khz * 1'000 * 2;
    }

    void get_gflops(double& out_gflops) noexcept override
    {
        const double lanes = static_cast<double>(m_properties.getGpuEuCount()) *
                             static_cast<double>(m_properties.getGpuEuSimdWidth());
        out_gflops =
            lanes * 2.0 * static_cast<double>(m_properties.getMaxClockFrequency()) / 1000.0;
    }

    void get_hardware_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, m_properties.getName().data());
    }

    void get_device_vendor(const ice::sonic::String& out_vendor) noexcept override
    {
        m_status.copy_into(out_vendor, m_properties.getVendor().data());
    }

    void get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) noexcept override
    {
        m_status.copy_into(out_pci_bus_id, m_pci_bus_id);
    }

    void get_device_properties(
        ice::sonic::DeviceProperties* out_properties,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        static_cast<void>(out_status);
        *out_properties = m_properties;
    }

    void get_native_handle(void** out_handle) noexcept override
    {
        *out_handle = &m_device;
    }

    const sycl::device& getNativeDevice() const noexcept
    {
        return m_device;
    }

    int getDeviceIndex() const noexcept
    {
        return m_device_index;
    }

    int32_t getGlobalIndex() const noexcept
    {
        return m_global_index;
    }

    const ice::sonic::DeviceProperties& getProperties() const noexcept
    {
        return m_properties;
    }

private:
    int32_t resolve_global_index(const sycl::device& device) const
    {
        const auto all_devices = sycl::device::get_devices();
        const auto found = std::ranges::find(all_devices, device);
        return found == all_devices.end()
                   ? -1
                   : static_cast<int32_t>(std::distance(all_devices.begin(), found));
    }

    template<std::size_t Extent>
    void copy_bounded(const std::string& source, std::span<char, Extent> destination) const noexcept
    {
        const std::size_t count = std::min(source.size(), Extent - 1);
        std::copy_n(source.data(), count, destination.begin());
        destination[count] = '\0';
    }

    template<typename Info, typename Fallback>
    auto query_intel(sycl::aspect aspect, Fallback fallback) const
    {
        return m_device.has(aspect) ? m_device.get_info<Info>() : fallback;
    }

    uint32_t fp_config_bits(const std::vector<sycl::info::fp_config>& configs) const
    {
        uint32_t bits = 0;
        for (const auto config: configs) {
            bits |= 1U << static_cast<uint32_t>(config);
        }
        return bits;
    }

    void fill_properties()
    {
        namespace info = sycl::info::device;
        namespace intel = sycl::ext::intel::info::device;
        namespace oneapi = sycl::ext::oneapi::experimental;

        m_properties = ice::sonic::DeviceProperties{};
        auto& properties = m_properties;

        copy_bounded(m_device.get_info<info::name>(), properties.getName());
        copy_bounded(m_device.get_info<info::vendor>(), properties.getVendor());
        copy_bounded(m_device.get_info<info::driver_version>(), properties.getDriverVersion());
        copy_bounded(m_device.get_info<info::version>(), properties.getVersion());

        properties.setDeviceType(static_cast<int32_t>(m_device.get_info<info::device_type>()));
        properties.setIsAvailable(m_device.get_info<info::is_available>());
        properties.setArchitecture(
            static_cast<int32_t>(m_device.get_info<oneapi::info::device::architecture>())
        );

        properties.setGlobalMemSize(m_device.get_info<info::global_mem_size>());
        properties.setLocalMemSize(m_device.get_info<info::local_mem_size>());
        properties.setMaxMemAllocSize(m_device.get_info<info::max_mem_alloc_size>());
        properties.setGlobalMemCacheSize(m_device.get_info<info::global_mem_cache_size>());
        properties.setGlobalMemCacheLineSize(m_device.get_info<info::global_mem_cache_line_size>());
        properties.setGlobalMemCacheType(
            static_cast<uint32_t>(m_device.get_info<info::global_mem_cache_type>())
        );
        properties.setLocalMemType(
            static_cast<uint32_t>(m_device.get_info<info::local_mem_type>())
        );
        properties.setMemBaseAddrAlign(m_device.get_info<info::mem_base_addr_align>());

        properties.setMaxComputeUnits(m_device.get_info<info::max_compute_units>());
        properties.setMaxWorkItemDimensions(m_device.get_info<info::max_work_item_dimensions>());
        properties.setMaxWorkGroupSize(
            static_cast<uint32_t>(m_device.get_info<info::max_work_group_size>())
        );
        properties.setMaxNumSubGroups(m_device.get_info<info::max_num_sub_groups>());
        properties.setMaxClockFrequency(m_device.get_info<info::max_clock_frequency>());
        properties.setAddressBits(m_device.get_info<info::address_bits>());
        properties.setMaxParameterSize(
            static_cast<uint32_t>(m_device.get_info<info::max_parameter_size>())
        );
        properties.setPartitionMaxSubDevices(m_device.get_info<info::partition_max_sub_devices>());
        properties.setProfilingTimerResolution(
            m_device.get_info<info::profiling_timer_resolution>()
        );

        properties.setHalfFpConfig(fp_config_bits(m_device.get_info<info::half_fp_config>()));
        properties.setSingleFpConfig(fp_config_bits(m_device.get_info<info::single_fp_config>()));
        properties.setDoubleFpConfig(fp_config_bits(m_device.get_info<info::double_fp_config>()));

        properties.setPreferredVectorWidthChar(
            m_device.get_info<info::preferred_vector_width_char>()
        );
        properties.setPreferredVectorWidthShort(
            m_device.get_info<info::preferred_vector_width_short>()
        );
        properties.setPreferredVectorWidthInt(
            m_device.get_info<info::preferred_vector_width_int>()
        );
        properties.setPreferredVectorWidthLong(
            m_device.get_info<info::preferred_vector_width_long>()
        );
        properties.setPreferredVectorWidthFloat(
            m_device.get_info<info::preferred_vector_width_float>()
        );
        properties.setPreferredVectorWidthDouble(
            m_device.get_info<info::preferred_vector_width_double>()
        );
        properties.setPreferredVectorWidthHalf(
            m_device.get_info<info::preferred_vector_width_half>()
        );
        properties.setNativeVectorWidthChar(m_device.get_info<info::native_vector_width_char>());
        properties.setNativeVectorWidthShort(m_device.get_info<info::native_vector_width_short>());
        properties.setNativeVectorWidthInt(m_device.get_info<info::native_vector_width_int>());
        properties.setNativeVectorWidthLong(m_device.get_info<info::native_vector_width_long>());
        properties.setNativeVectorWidthFloat(m_device.get_info<info::native_vector_width_float>());
        properties.setNativeVectorWidthDouble(
            m_device.get_info<info::native_vector_width_double>()
        );
        properties.setNativeVectorWidthHalf(m_device.get_info<info::native_vector_width_half>());

        properties.setGpuEuCount(
            query_intel<intel::gpu_eu_count>(sycl::aspect::ext_intel_gpu_eu_count, 512U)
        );
        properties.setGpuEuCountPerSubslice(
            query_intel<intel::gpu_eu_count_per_subslice>(
                sycl::aspect::ext_intel_gpu_eu_count_per_subslice,
                8U
            )
        );
        properties.setGpuEuSimdWidth(
            query_intel<intel::gpu_eu_simd_width>(sycl::aspect::ext_intel_gpu_eu_simd_width, 8U)
        );
        properties.setGpuHwThreadsPerEu(
            query_intel<intel::gpu_hw_threads_per_eu>(
                sycl::aspect::ext_intel_gpu_hw_threads_per_eu,
                8U
            )
        );
        properties.setDeviceId(
            static_cast<int32_t>(
                query_intel<intel::device_id>(sycl::aspect::ext_intel_device_id, 0U)
            )
        );
        properties.setMemoryClockRate(
            query_intel<intel::memory_clock_rate>(sycl::aspect::ext_intel_memory_clock_rate, 0U)
        );
        properties.setMemoryBusWidth(
            query_intel<intel::memory_bus_width>(sycl::aspect::ext_intel_memory_bus_width, 0U)
        );

        if (m_device.has(sycl::aspect::ext_intel_device_info_uuid)) {
            const auto uuid = m_device.get_info<intel::uuid>();
            std::ranges::copy(uuid, properties.getUuid().begin());
        }
        if (m_device.has(sycl::aspect::ext_intel_pci_address)) {
            m_pci_bus_id = m_device.get_info<intel::pci_address>();
        }

        properties.setXeStackCount(1);
        properties.setXeRegionsPerStack(1);
        properties.setXeClustersPerRegion(1);
        properties.setXeCoresPerCluster(64);
        properties.setEusPerXeCore(8);
        properties.setMaxLanesPerHwThread(32);

        const auto sub_group_sizes = m_device.get_info<info::sub_group_sizes>();
        properties.setNumSubGroupSizes(
            static_cast<uint32_t>(
                std::min(sub_group_sizes.size(), properties.getSubGroupSizes().size())
            )
        );
        for (uint32_t index = 0; index < properties.getNumSubGroupSizes(); ++index) {
            properties.getSubGroupSizes()[index] = static_cast<uint32_t>(sub_group_sizes[index]);
        }

        properties.setHasFp16(m_device.has(sycl::aspect::fp16));
        properties.setHasFp64(m_device.has(sycl::aspect::fp64));
        properties.setHasAtomic64(m_device.has(sycl::aspect::atomic64));
        properties.setHasBfloat16Conversions(
            m_device.has(sycl::aspect::ext_oneapi_bfloat16_math_functions)
        );
        properties.setHasSubgroupMatrixMultiplyAccumulate(
            m_device.has(sycl::aspect::ext_intel_matrix)
        );
        properties.setHasSubgroupMatrixMultiplyAccumulateTensorFloat32(
            properties.getHasSubgroupMatrixMultiplyAccumulate()
        );
        properties.setHasSubgroup2dBlockIo(m_device.has(sycl::aspect::ext_intel_matrix));
    }

    SyclStatus m_status;
    sycl::device m_device;
    int m_device_index{-1};
    int32_t m_global_index{-1};
    std::string m_pci_bus_id;
    ice::sonic::DeviceProperties m_properties;
};

} // namespace aten_xpu
