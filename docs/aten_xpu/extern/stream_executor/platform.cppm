module;

#include "include/c/extern/stream_executor/platform.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:platform;

import std;
import cc_ice_intern_sonic;
import cc_ice_extern_stream_executor_builder;
import aten_xpu_intern;
import :device;
import :executor;
import :peer_access;
import :level_zero;

export namespace aten_xpu {

class SyclPlatform : public ice::builder::TF_PlatformOps
{
public:
    explicit SyclPlatform(const SyclOpsTable& ops) :
        ice::builder::TF_PlatformOps{ops.getDeviceOps(), ops.getExecutorOps(), ops.getStatusOps()},
        m_status{ops}
    {

        enumerate_devices();

    }

    ~SyclPlatform() override = default;
    SyclPlatform(const SyclPlatform&) = delete;
    SyclPlatform& operator=(const SyclPlatform&) = delete;
    SyclPlatform(SyclPlatform&&) = delete;
    SyclPlatform& operator=(SyclPlatform&&) = delete;

    static void create(::TF_Platform* handle)
    {

        auto* platform = new SyclPlatform{SyclOpsTable::getInstance()};
        SyclHandle::attach(handle, *platform);

    }

    void destroy() noexcept override { delete this; }

    void get_device_count(int* out_device_count, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(out_status);
        *out_device_count = static_cast<int>(m_native_devices.size());

    }

    void create_device_internal(
        const ice::sonic::TF_DeviceOps& device,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (m_native_devices.empty()) {
            m_status.fail(out_status, TF_NOT_FOUND, "no XPU device available");
            return;
        }

        const auto index = m_next_device_index % m_native_devices.size();
        ++m_next_device_index;
        try {
            SyclHandle::resolve<SyclDevice>(device).bind(m_native_devices[index], static_cast<int>(index));
        } catch (const sycl::exception& error) {
            m_status.fail_from(out_status, error);
        }

    }

    void destroy_device_internal(const ice::sonic::TF_DeviceOps& device) noexcept override
    {

        static_cast<void>(device);

    }

    void create_executor_internal(
        const ice::sonic::TF_ExecutorOps& executor,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        static_cast<void>(out_status);
        SyclHandle::resolve<SyclExecutor>(executor).bind(
            m_context,
            [this](int device_index, int peer_device_index)
            {

                return enable_peer_access(device_index, peer_device_index);

            }
        );

    }

    void destroy_executor_internal(const ice::sonic::TF_ExecutorOps& executor) noexcept override
    {

        SyclHandle::resolve<SyclExecutor>(executor).release();

    }

    void get_current_device(int* out_device_index, const ice::sonic::Status& out_status)
        noexcept override
    {

        static_cast<void>(out_status);
        *out_device_index = s_current_device_index;

    }

    void set_current_device(int device_index, const ice::sonic::Status& out_status)
        noexcept override
    {

        if (!is_valid_index(device_index)) {
            m_status.fail(out_status, TF_OUT_OF_RANGE, "device index out of range");
            return;
        }
        s_current_device_index = device_index;

    }

    void get_device_for_pointer(
        const void* pointer,
        int* out_device_index,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        const auto index = find_pointer_device(pointer);
        if (!index) {
            m_status.fail(out_status, TF_NOT_FOUND, "pointer does not belong to this platform");
            return;
        }
        *out_device_index = *index;

    }

    void can_access_peer(
        int device_index,
        int peer_device_index,
        _Bool* out_can_access,
        const ice::sonic::Status& out_status
    ) noexcept override
    {

        if (!is_valid_index(device_index) || !is_valid_index(peer_device_index)) {
            m_status.fail(out_status, TF_OUT_OF_RANGE, "device index out of range");
            return;
        }

        const auto index = static_cast<std::size_t>(device_index);
        const auto peer_index = static_cast<std::size_t>(peer_device_index);
        *out_can_access = m_peer_access.can_access(
            m_native_devices[index],
            index,
            m_native_devices[peer_index],
            peer_index
        );

    }

    void get_native_handle(void** out_handle) noexcept override { *out_handle = &m_context; }

    int exchange_device(int device_index)
    {

        const int previous = s_current_device_index;
        if (device_index != previous && is_valid_index(device_index)) {
            s_current_device_index = device_index;
        }
        return previous;

    }

    bool enable_peer_access(int device_index, int peer_device_index)
    {

        if (!is_valid_index(device_index) || !is_valid_index(peer_device_index)) {
            return false;
        }

        const auto index = static_cast<std::size_t>(device_index);
        const auto peer_index = static_cast<std::size_t>(peer_device_index);
        return m_peer_access.enable(
            m_native_devices[index],
            index,
            m_native_devices[peer_index],
            peer_index
        );

    }

    const sycl::context& getContext() const noexcept { return m_context; }

    const sycl::device& getNativeDevice(int device_index) const
    {

        return m_native_devices.at(static_cast<std::size_t>(device_index));

    }

    int getDeviceCount() const noexcept { return static_cast<int>(m_native_devices.size()); }

private:
    bool is_valid_index(int device_index) const noexcept
    {

        return device_index >= 0 && static_cast<std::size_t>(device_index) < m_native_devices.size();

    }

    std::optional<int> find_pointer_device(const void* pointer) const
    {

        try {
            const auto owner = sycl::get_pointer_device(pointer, m_context);
            const auto found = std::ranges::find(m_native_devices, owner);
            if (found != m_native_devices.end()) {
                return static_cast<int>(std::distance(m_native_devices.begin(), found));
            }
        } catch (const sycl::exception&) {
            const auto native = SyclLevelZero::getInstance().query_allocation_device(m_context, pointer);
            if (!native) {
                return std::nullopt;
            }
            for (std::size_t index = 0; index < m_native_devices.size(); ++index) {
                if (sycl::get_native<sycl::backend::ext_oneapi_level_zero>(m_native_devices[index]) ==
                    *native)
                {
                    return static_cast<int>(index);
                }
            }
        }
        return std::nullopt;

    }

    static bool is_integrated(const sycl::device& device)
    {

        return device.has(sycl::aspect::ext_oneapi_is_integrated_gpu);

    }

    static bool has_gpu(const sycl::platform& platform, bool want_integrated)
    {

        if (platform.get_backend() != sycl::backend::ext_oneapi_level_zero) {
            return false;
        }
        return std::ranges::any_of(
            platform.get_devices(),
            [want_integrated](const sycl::device& device)
            {

                return device.is_gpu() && is_integrated(device) == want_integrated;

            }
        );

    }

    bool collect_devices(const std::vector<sycl::platform>& platforms, bool want_integrated)
    {

        for (const auto& platform: platforms) {
            if (!has_gpu(platform, want_integrated)) {
                continue;
            }
            for (const auto& device: platform.get_devices()) {
                if (device.is_gpu() && (want_integrated || !is_integrated(device))) {
                    m_native_devices.push_back(device);
                }
            }
            return true;
        }
        return false;

    }

    void enumerate_devices()
    {

        try {
            const auto platforms = sycl::platform::get_platforms();
            if (!collect_devices(platforms, false)) {
                collect_devices(platforms, true);
            }
        } catch (const sycl::exception&) {
            m_native_devices.clear();
        }

        if (m_native_devices.empty()) {
            return;
        }
        m_context = m_native_devices.front().get_platform().khr_get_default_context();
        m_peer_access.reset(m_native_devices.size());

    }

    SyclStatus m_status;
    std::vector<sycl::device> m_native_devices;
    sycl::context m_context;
    SyclPeerAccess m_peer_access;
    std::size_t m_next_device_index{0};

    // Per calling thread, as the C header documents for get/set_current_device.
    static thread_local int s_current_device_index;
};

inline thread_local int SyclPlatform::s_current_device_index = 0;

} // namespace aten_xpu
