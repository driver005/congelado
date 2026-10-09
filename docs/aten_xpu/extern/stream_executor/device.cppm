module;

#include "include/c/extern/stream_executor/device.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:device;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;

export namespace aten_xpu {

class SyclDevice : public ice::builder::TF_DeviceOps
{
public:
    explicit SyclDevice(const SyclOpsTable& ops) noexcept :
        ice::builder::TF_DeviceOps{ops.getStatusOps(), ops.getStringOps()},
        m_status{ops}
    {
    }

    ~SyclDevice() override = default;
    SyclDevice(const SyclDevice&) = delete;
    SyclDevice& operator=(const SyclDevice&) = delete;
    SyclDevice(SyclDevice&&) = delete;
    SyclDevice& operator=(SyclDevice&&) = delete;

    static void create(::TF_Device* handle)
    {
        auto* device = new SyclDevice{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *device);
    }

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

    void get_numa_node(int32_t* out_numa_node) noexcept override
    {
        *out_numa_node = -1;
    }

    void get_memory_bandwidth(int64_t* out_bandwidth) noexcept override
    {
        const auto bus_bits = static_cast<int64_t>(m_properties.memory_bus_width);
        const auto clock_khz = static_cast<int64_t>(m_properties.memory_clock_rate);
        *out_bandwidth = bus_bits / 8 * clock_khz * 1'000 * 2;
    }

    void get_gflops(double* out_gflops) noexcept override
    {
        const double lanes = static_cast<double>(m_properties.gpu_eu_count) *
                             static_cast<double>(m_properties.gpu_eu_simd_width);
        *out_gflops = lanes * 2.0 * static_cast<double>(m_properties.max_clock_frequency) / 1000.0;
    }

    void get_hardware_name(const ice::sonic::String& out_name) noexcept override
    {
        m_status.copy_into(out_name, m_properties.name);
    }

    void get_device_vendor(const ice::sonic::String& out_vendor) noexcept override
    {
        m_status.copy_into(out_vendor, m_properties.vendor);
    }

    void get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) noexcept override
    {
        m_status.copy_into(out_pci_bus_id, m_pci_bus_id);
    }

    void get_device_properties(
        TF_DeviceProperties* out_properties,
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

    const TF_DeviceProperties& getProperties() const noexcept
    {
        return m_properties;
    }

private:
    static int32_t resolve_global_index(const sycl::device& device)
    {
        const auto all_devices = sycl::device::get_devices();
        const auto found = std::ranges::find(all_devices, device);
        return found == all_devices.end()
                   ? -1
                   : static_cast<int32_t>(std::distance(all_devices.begin(), found));
    }

    template<std::size_t Extent>
    static void copy_bounded(const std::string& source, char (&destination)[Extent]) noexcept
    {
        const std::size_t count = std::min(source.size(), Extent - 1);
        std::copy_n(source.data(), count, destination);
        destination[count] = '\0';
    }

    template<typename Info, typename Fallback>
    auto query_intel(sycl::aspect aspect, Fallback fallback) const
    {
        return m_device.has(aspect) ? m_device.get_info<Info>() : fallback;
    }

    static uint32_t fp_config_bits(const std::vector<sycl::info::fp_config>& configs)
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

        m_properties = TF_DeviceProperties{.struct_size = sizeof(TF_DeviceProperties)};
        auto& properties = m_properties;

        copy_bounded(m_device.get_info<info::name>(), properties.name);
        copy_bounded(m_device.get_info<info::vendor>(), properties.vendor);
        copy_bounded(m_device.get_info<info::driver_version>(), properties.driver_version);
        copy_bounded(m_device.get_info<info::version>(), properties.version);

        properties.device_type = static_cast<int32_t>(m_device.get_info<info::device_type>());
        properties.is_available = m_device.get_info<info::is_available>();
        properties.architecture =
            static_cast<int32_t>(m_device.get_info<oneapi::info::device::architecture>());

        properties.global_mem_size = m_device.get_info<info::global_mem_size>();
        properties.local_mem_size = m_device.get_info<info::local_mem_size>();
        properties.max_mem_alloc_size = m_device.get_info<info::max_mem_alloc_size>();
        properties.global_mem_cache_size = m_device.get_info<info::global_mem_cache_size>();
        properties.global_mem_cache_line_size =
            m_device.get_info<info::global_mem_cache_line_size>();
        properties.global_mem_cache_type =
            static_cast<uint32_t>(m_device.get_info<info::global_mem_cache_type>());
        properties.local_mem_type =
            static_cast<uint32_t>(m_device.get_info<info::local_mem_type>());
        properties.mem_base_addr_align = m_device.get_info<info::mem_base_addr_align>();

        properties.max_compute_units = m_device.get_info<info::max_compute_units>();
        properties.max_work_item_dimensions = m_device.get_info<info::max_work_item_dimensions>();
        properties.max_work_group_size =
            static_cast<uint32_t>(m_device.get_info<info::max_work_group_size>());
        properties.max_num_sub_groups = m_device.get_info<info::max_num_sub_groups>();
        properties.max_clock_frequency = m_device.get_info<info::max_clock_frequency>();
        properties.address_bits = m_device.get_info<info::address_bits>();
        properties.max_parameter_size =
            static_cast<uint32_t>(m_device.get_info<info::max_parameter_size>());
        properties.partition_max_sub_devices = m_device.get_info<info::partition_max_sub_devices>();
        properties.profiling_timer_resolution =
            m_device.get_info<info::profiling_timer_resolution>();

        properties.half_fp_config = fp_config_bits(m_device.get_info<info::half_fp_config>());
        properties.single_fp_config = fp_config_bits(m_device.get_info<info::single_fp_config>());
        properties.double_fp_config = fp_config_bits(m_device.get_info<info::double_fp_config>());

        properties.preferred_vector_width_char =
            m_device.get_info<info::preferred_vector_width_char>();
        properties.preferred_vector_width_short =
            m_device.get_info<info::preferred_vector_width_short>();
        properties.preferred_vector_width_int =
            m_device.get_info<info::preferred_vector_width_int>();
        properties.preferred_vector_width_long =
            m_device.get_info<info::preferred_vector_width_long>();
        properties.preferred_vector_width_float =
            m_device.get_info<info::preferred_vector_width_float>();
        properties.preferred_vector_width_double =
            m_device.get_info<info::preferred_vector_width_double>();
        properties.preferred_vector_width_half =
            m_device.get_info<info::preferred_vector_width_half>();
        properties.native_vector_width_char = m_device.get_info<info::native_vector_width_char>();
        properties.native_vector_width_short = m_device.get_info<info::native_vector_width_short>();
        properties.native_vector_width_int = m_device.get_info<info::native_vector_width_int>();
        properties.native_vector_width_long = m_device.get_info<info::native_vector_width_long>();
        properties.native_vector_width_float = m_device.get_info<info::native_vector_width_float>();
        properties.native_vector_width_double =
            m_device.get_info<info::native_vector_width_double>();
        properties.native_vector_width_half = m_device.get_info<info::native_vector_width_half>();

        properties.gpu_eu_count =
            query_intel<intel::gpu_eu_count>(sycl::aspect::ext_intel_gpu_eu_count, 512U);
        properties.gpu_eu_count_per_subslice = query_intel<intel::gpu_eu_count_per_subslice>(
            sycl::aspect::ext_intel_gpu_eu_count_per_subslice,
            8U
        );
        properties.gpu_eu_simd_width =
            query_intel<intel::gpu_eu_simd_width>(sycl::aspect::ext_intel_gpu_eu_simd_width, 8U);
        properties.gpu_hw_threads_per_eu = query_intel<intel::gpu_hw_threads_per_eu>(
            sycl::aspect::ext_intel_gpu_hw_threads_per_eu,
            8U
        );
        properties.device_id = static_cast<int32_t>(
            query_intel<intel::device_id>(sycl::aspect::ext_intel_device_id, 0U)
        );
        properties.memory_clock_rate =
            query_intel<intel::memory_clock_rate>(sycl::aspect::ext_intel_memory_clock_rate, 0U);
        properties.memory_bus_width =
            query_intel<intel::memory_bus_width>(sycl::aspect::ext_intel_memory_bus_width, 0U);

        if (m_device.has(sycl::aspect::ext_intel_device_info_uuid)) {
            const auto uuid = m_device.get_info<intel::uuid>();
            std::ranges::copy(uuid, std::begin(properties.uuid));
        }
        if (m_device.has(sycl::aspect::ext_intel_pci_address)) {
            m_pci_bus_id = m_device.get_info<intel::pci_address>();
        }

        properties.xe_stack_count = 1;
        properties.xe_regions_per_stack = 1;
        properties.xe_clusters_per_region = 1;
        properties.xe_cores_per_cluster = 64;
        properties.eus_per_xe_core = 8;
        properties.max_lanes_per_hw_thread = 32;

        const auto sub_group_sizes = m_device.get_info<info::sub_group_sizes>();
        properties.num_sub_group_sizes = static_cast<uint32_t>(
            std::min(sub_group_sizes.size(), std::size(properties.sub_group_sizes))
        );
        for (uint32_t index = 0; index < properties.num_sub_group_sizes; ++index) {
            properties.sub_group_sizes[index] = static_cast<uint32_t>(sub_group_sizes[index]);
        }

        properties.has_fp16 = m_device.has(sycl::aspect::fp16);
        properties.has_fp64 = m_device.has(sycl::aspect::fp64);
        properties.has_atomic64 = m_device.has(sycl::aspect::atomic64);
        properties.has_bfloat16_conversions =
            m_device.has(sycl::aspect::ext_oneapi_bfloat16_math_functions);
        properties.has_subgroup_matrix_multiply_accumulate =
            m_device.has(sycl::aspect::ext_intel_matrix);
        properties.has_subgroup_matrix_multiply_accumulate_tensor_float32 =
            properties.has_subgroup_matrix_multiply_accumulate;
        properties.has_subgroup_2d_block_io = m_device.has(sycl::aspect::ext_intel_matrix);
    }

    SyclStatus m_status;
    sycl::device m_device;
    int m_device_index{-1};
    int32_t m_global_index{-1};
    std::string m_pci_bus_id;
    TF_DeviceProperties m_properties{};
};

} // namespace aten_xpu
