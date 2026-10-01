// SYCL reference plugin — device metadata.
//
// Not built by Bazel (docs/ only). One SyclDevice per enumerated sycl::device, owned by
// SyclPlatform's device pool. See platform.cppm for the enumeration order (dGPU before iGPU)
// and the shared sycl::context every device and executor uses.

module;

#include "include/c/extern/stream_executor/device.h"

export module sycl_backend:device;

import std;
import cc_ice_extern_stream_executor_builder;

export namespace sycl_backend {

class SyclDevice : public ice::builder::Device
{
public:
    explicit SyclDevice(sycl::device&& device) noexcept :
        m_device{std::move(device)}
    {
    }

    ~SyclDevice() override = default;
    SyclDevice(const SyclDevice&) = delete;
    SyclDevice& operator=(const SyclDevice&) = delete;
    SyclDevice(SyclDevice&&) = delete;
    SyclDevice& operator=(SyclDevice&&) = delete;

    const sycl::device& get_native_device() const noexcept
    {
        return m_device;
    }

    void get_numa_node(int32_t* out_numa_node) noexcept override
    {
        // SYCL has no portable NUMA query; the caching allocator does not need it either. Report
        // "unknown" rather than guessing.
        *out_numa_node = -1;
    }

    void get_memory_bandwidth(int64_t* out_bandwidth) noexcept override
    {
        // Not exposed by core SYCL. c10/XPUDeviceProp.h leaves the analogous field at 0 too.
        *out_bandwidth = 0;
    }

    void get_gflops(double* out_gflops) noexcept override
    {
        *out_gflops = 0.0;
    }

    void get_hardware_name(ice::builder::String& out_name) noexcept override
    {
        copy_into(out_name, m_device.get_info<sycl::info::device::name>());
    }

    void get_device_vendor(ice::builder::String& out_vendor) noexcept override
    {
        copy_into(out_vendor, m_device.get_info<sycl::info::device::vendor>());
    }

    void get_pci_bus_id(ice::builder::String& out_pci_bus_id) noexcept override
    {
        if (m_device.has(sycl::aspect::ext_intel_pci_address)) {
            copy_into(
                out_pci_bus_id,
                m_device.get_info<sycl::ext::intel::info::device::pci_address>()
            );
        } else {
            copy_into(out_pci_bus_id, std::string{});
        }
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_device_properties(TF_DeviceProperties* out_properties) noexcept override
    {
        namespace syclex = sycl::ext::oneapi::experimental;

        *out_properties = TF_DeviceProperties{.struct_size = TF_DEVICE_PROPERTIES_STRUCT_SIZE};

        copy_bounded(m_device.get_info<sycl::info::device::name>(), out_properties->name);
        copy_bounded(m_device.get_info<sycl::info::device::vendor>(), out_properties->vendor);
        copy_bounded(
            m_device.get_info<sycl::info::device::driver_version>(),
            out_properties->driver_version
        );
        copy_bounded(m_device.get_info<sycl::info::device::version>(), out_properties->version);

        out_properties->device_type = static_cast<int32_t>(m_device.is_gpu() ? 1 : 0);
        out_properties->is_available = true;
        out_properties->architecture =
            static_cast<int32_t>(m_device.get_info<syclex::info::device::architecture>());

        out_properties->global_mem_size = m_device.get_info<sycl::info::device::global_mem_size>();
        out_properties->local_mem_size = m_device.get_info<sycl::info::device::local_mem_size>();
        out_properties->max_mem_alloc_size =
            m_device.get_info<sycl::info::device::max_mem_alloc_size>();
        out_properties->global_mem_cache_size =
            m_device.get_info<sycl::info::device::global_mem_cache_size>();
        out_properties->global_mem_cache_line_size =
            m_device.get_info<sycl::info::device::global_mem_cache_line_size>();
        out_properties->mem_base_addr_align =
            m_device.get_info<sycl::info::device::mem_base_addr_align>();

        out_properties->max_compute_units =
            m_device.get_info<sycl::info::device::max_compute_units>();
        out_properties->max_work_item_dimensions =
            m_device.get_info<sycl::info::device::max_work_item_dimensions>();
        out_properties->max_work_group_size =
            static_cast<uint32_t>(m_device.get_info<sycl::info::device::max_work_group_size>());
        out_properties->max_clock_frequency =
            m_device.get_info<sycl::info::device::max_clock_frequency>();
        out_properties->address_bits = m_device.get_info<sycl::info::device::address_bits>();
        out_properties->max_parameter_size =
            static_cast<uint32_t>(m_device.get_info<sycl::info::device::max_parameter_size>());
        out_properties->partition_max_sub_devices =
            m_device.get_info<sycl::info::device::partition_max_sub_devices>();
        out_properties->profiling_timer_resolution =
            m_device.get_info<sycl::info::device::profiling_timer_resolution>();

        if (m_device.has(sycl::aspect::ext_intel_device_id)) {
            out_properties->device_id = static_cast<int32_t>(
                m_device.get_info<sycl::ext::intel::info::device::device_id>()
            );
        }

        fill_sub_group_sizes(*out_properties);
        fill_capability_flags(*out_properties);

        return {};
    }

    void get_native_handle(void** out_handle) noexcept override
    {
        *out_handle = &m_device;
    }

private:
    // TF_StringOps::copy has the plugin behind out_name make its own internal copy of src/size.
    static void copy_into(ice::builder::String& target, const std::string& source) noexcept
    {
        target.copy(source.data(), source.size());
    }

    // TF_DeviceProperties::name/vendor/... are fixed-size char arrays; a driver string longer than
    // that is truncated rather than overflowing.
    template<std::size_t Extent>
    static void copy_bounded(const std::string& source, char (&destination)[Extent]) noexcept
    {
        const std::size_t count = std::min(source.size(), Extent - 1);
        std::copy_n(source.data(), count, destination);
        destination[count] = '\0';
    }

    void fill_sub_group_sizes(TF_DeviceProperties& properties) const
    {
        const std::vector<std::size_t> sizes =
            m_device.get_info<sycl::info::device::sub_group_sizes>();

        properties.num_sub_group_sizes =
            static_cast<uint32_t>(std::min(sizes.size(), std::size(properties.sub_group_sizes)));

        for (uint32_t index = 0; index < properties.num_sub_group_sizes; ++index) {
            properties.sub_group_sizes[index] = static_cast<uint32_t>(sizes[index]);
        }
    }

    void fill_capability_flags(TF_DeviceProperties& properties) const
    {
        properties.has_fp16 = m_device.has(sycl::aspect::fp16);
        properties.has_fp64 = m_device.has(sycl::aspect::fp64);
        properties.has_atomic64 = m_device.has(sycl::aspect::atomic64);

        namespace syclex = sycl::ext::oneapi::experimental;
        properties.has_subgroup_matrix_multiply_accumulate =
            m_device.has(syclex::aspect::ext_oneapi_matrix);
        properties.has_subgroup_matrix_multiply_accumulate_tensor_float32 =
            properties.has_subgroup_matrix_multiply_accumulate;
        properties.has_subgroup_2d_block_io = m_device.has(syclex::aspect::ext_oneapi_tensor_map);
    }

    sycl::device m_device;
};

} // namespace sycl_backend
