// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/device.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/device.h"

export module cc_ice_extern_stream_executor_sonic:device;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_DeviceOps : public ice::sonic::Runtime<TF_DeviceOps, TF_DeviceOps>
{
public:
    explicit TF_DeviceOps(TF_DeviceOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "stream_executor";

    [[nodiscard]] std::expected<void, ice::Status> get_numa_node(int32_t* out_numa_node) noexcept
    {
        ice::Status status;
        m_ops->get_numa_node(get_handle(), out_numa_node, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_memory_bandwidth(int64_t* out_bandwidth) noexcept
    {
        ice::Status status;
        m_ops->get_memory_bandwidth(get_handle(), out_bandwidth, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_gflops(double* out_gflops) noexcept
    {
        ice::Status status;
        m_ops->get_gflops(get_handle(), out_gflops, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_hardware_name(const ice::sonic::String& out_name) noexcept
    {
        ice::Status status;
        m_ops->get_hardware_name(get_handle(), out_name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_device_vendor(const ice::sonic::String& out_vendor) noexcept
    {
        ice::Status status;
        m_ops->get_device_vendor(get_handle(), out_vendor.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_pci_bus_id(const ice::sonic::String& out_pci_bus_id) noexcept
    {
        ice::Status status;
        m_ops->get_pci_bus_id(get_handle(), out_pci_bus_id.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_device_properties(TF_DeviceProperties* out_properties) noexcept
    {
        ice::Status status;
        m_ops->get_device_properties(get_handle(), out_properties, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_native_handle(void** out_handle) noexcept
    {
        ice::Status status;
        m_ops->get_native_handle(get_handle(), out_handle, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
