// SYCL reference plugin — device pool, shared context, current-device-per-thread, P2P.
//
// Not built by Bazel (docs/ only). Replaces c10/XPUFunctions.cpp + c10/PeerToPeerAccess.cpp.
//
// ASSUMED FIX to the generator: create_device_internal/create_executor_internal are declared
// taking an already-dereferenced ice::builder::Device&/Executor& (Device::create(handle), i.e.
// static_cast<Device*>(handle->plugin_data)), but plugin_data is null before this call — that is
// the whole point of a "create the handle" slot. This file overrides them taking the raw
// TF_Device*/TF_Executor* instead, so the plugin can allocate its own object and assign
// handle->plugin_data itself. Once the generator's Builder-tier signatures are fixed to match,
// only the override signatures here need to change, not the logic.

module;

#include "include/c/extern/stream_executor/platform.h"

export module sycl_backend:platform;

import std;
import cc_ice_extern_stream_executor_builder;
import :device;
import :executor;

export namespace sycl_backend {

class SyclPlatform : public ice::builder::Platform
{
public:
    SyclPlatform() noexcept
    {
        enumerate_devices();
    }

    ~SyclPlatform() override = default;
    SyclPlatform(const SyclPlatform&) = delete;
    SyclPlatform& operator=(const SyclPlatform&) = delete;
    SyclPlatform(SyclPlatform&&) = delete;
    SyclPlatform& operator=(SyclPlatform&&) = delete;

    // The default context every SyclDevice, SyclExecutor and oneDNN engine in this plugin shares.
    // c10/XPUFunctions.cpp builds this the same way: from the platform of the first enumerated
    // device, after enumeration decided which platform (dGPU's or iGPU's) to use.
    const sycl::context& get_shared_context() const noexcept
    {
        return m_context;
    }

    SyclDevice& get_device_by_index(int device_index)
    {
        return *m_devices.at(static_cast<std::size_t>(device_index));
    }

    int get_index_of(const SyclDevice& device) const noexcept
    {
        for (std::size_t index = 0; index < m_devices.size(); ++index) {
            if (m_devices[index].get() == &device) {
                return static_cast<int>(index);
            }
        }
        return 0;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_device_count(int* out_device_count) noexcept override
    {
        *out_device_count = static_cast<int>(m_devices.size());
        return {};
    }

    // See the file-level note: takes the raw handle, not a dereferenced Device&.
    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create_device_internal(TF_Device* device) noexcept
    {
        if (m_next_device_index >= m_devices.size()) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclPlatform: no more devices to hand out")
            };
        }

        device->plugin_data = m_devices[m_next_device_index].get();
        ++m_next_device_index;
        return {};
    }

    void destroy_device_internal(TF_Device* device) noexcept
    {
        // Devices are owned by m_devices for the platform's whole lifetime; nothing to free here.
        device->plugin_data = nullptr;
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create_executor_internal(TF_Executor* executor) noexcept
    {
        auto owned = std::make_unique<SyclExecutor>(*this);
        executor->plugin_data = owned.get();
        m_executors.push_back(std::move(owned));
        return {};
    }

    void destroy_executor_internal(TF_Executor* executor) noexcept
    {
        auto found = std::ranges::find_if(
            m_executors,
            [executor](const std::unique_ptr<SyclExecutor>& candidate)
            {
                return candidate.get() == executor->plugin_data;
            }
        );

        if (found != m_executors.end()) {
            m_executors.erase(found);
        }

        executor->plugin_data = nullptr;
    }

    // Per calling thread, per the C header's documented contract.
    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_current_device(int* out_device_index) noexcept override
    {
        *out_device_index = s_current_device_index;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_current_device(int device_index) noexcept override
    {
        if (device_index < 0 || static_cast<std::size_t>(device_index) >= m_devices.size()) {
            return std::unexpected{
                ice::sonic::Status::from_message("SyclPlatform: device index out of range")
            };
        }

        s_current_device_index = device_index;
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_device_for_pointer(const void* pointer, int* out_device_index) noexcept override
    {
        auto* owning_device = sycl::get_pointer_device(const_cast<void*>(pointer), m_context);

        for (std::size_t index = 0; index < m_devices.size(); ++index) {
            if (m_devices[index]->get_native_device() == owning_device) {
                *out_device_index = static_cast<int>(index);
                return {};
            }
        }

        return std::unexpected{ice::sonic::Status::from_message(
            "SyclPlatform: pointer does not belong to this platform"
        )};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    can_access_peer(int device_index, int peer_device_index, bool* out_can_access) noexcept override
    {
        const auto key = std::pair{device_index, peer_device_index};

        if (auto cached = m_peer_access_cache.find(key); cached != m_peer_access_cache.end()) {
            *out_can_access = cached->second;
            return {};
        }

        const sycl::device& device =
            m_devices.at(static_cast<std::size_t>(device_index))->get_native_device();
        const sycl::device& peer =
            m_devices.at(static_cast<std::size_t>(peer_device_index))->get_native_device();

        const bool can_access = device.ext_oneapi_can_access_peer(
            peer,
            sycl::ext::oneapi::peer_access::access_supported
        );

        m_peer_access_cache.emplace(key, can_access);
        *out_can_access = can_access;
        return {};
    }

    void get_native_handle(void** out_handle) noexcept override
    {
        *out_handle = const_cast<sycl::context*>(&m_context);
    }

private:
    // See Note [Device Management] in c10/XPUFunctions.cpp: prefer the first platform that has at
    // least one dGPU, enumerating only its dGPUs; only fall back to the first platform with an
    // iGPU if no dGPU platform exists. sycl::context cannot span platforms, so mixing is not an
    // option.
    void enumerate_devices()
    {
        const auto platforms = sycl::platform::get_platforms();

        auto is_integrated = [](const sycl::device& device)
        {
            return device.has(sycl::aspect::ext_oneapi_is_integrated_gpu);
        };

        auto platform_has_gpu =
            [&is_integrated](const sycl::platform& platform, bool want_integrated)
        {
            if (platform.get_backend() != sycl::backend::ext_oneapi_level_zero) {
                return false;
            }

            for (const sycl::device& device: platform.get_devices()) {
                if (device.is_gpu() && (is_integrated(device) == want_integrated)) {
                    return true;
                }
            }

            return false;
        };

        for (const sycl::platform& platform: platforms) {
            if (!platform_has_gpu(platform, false)) {
                continue;
            }

            for (sycl::device device: platform.get_devices()) {
                if (device.is_gpu() && !is_integrated(device)) {
                    m_devices.push_back(std::make_unique<SyclDevice>(std::move(device)));
                }
            }

            break;
        }

        if (m_devices.empty()) {
            for (const sycl::platform& platform: platforms) {
                if (!platform_has_gpu(platform, true)) {
                    continue;
                }

                for (sycl::device device: platform.get_devices()) {
                    if (device.is_gpu()) {
                        m_devices.push_back(std::make_unique<SyclDevice>(std::move(device)));
                    }
                }

                break;
            }
        }

        if (m_devices.empty()) {
            return;
        }

        m_context = sycl::context{
            m_devices.front()->get_native_device().get_platform().khr_get_default_context()
        };
    }

    std::vector<std::unique_ptr<SyclDevice>> m_devices;
    std::vector<std::unique_ptr<SyclExecutor>> m_executors;
    std::size_t m_next_device_index{0};
    sycl::context m_context;
    std::map<std::pair<int, int>, bool> m_peer_access_cache;

    // Per calling thread, matching c10/XPUFunctions.cpp's thread_local curDeviceIndex — this is
    // the vtable's own documented contract, not an ATen-side workaround.
    static thread_local int s_current_device_index;
};

inline thread_local int SyclPlatform::s_current_device_index = 0;

} // namespace sycl_backend
