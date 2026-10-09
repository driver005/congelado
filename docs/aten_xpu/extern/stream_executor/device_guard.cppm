module;

export module aten_xpu_extern_stream_executor:device_guard;

import std;
import :platform;

export namespace aten_xpu {

class SyclDeviceGuard
{
public:
    SyclDeviceGuard(SyclPlatform& platform, int device_index) :
        m_platform{platform},
        m_original_index{platform.exchange_device(device_index)}
    {
    }

    ~SyclDeviceGuard()
    {
        m_platform.exchange_device(m_original_index);
    }

    SyclDeviceGuard(const SyclDeviceGuard&) = delete;
    SyclDeviceGuard& operator=(const SyclDeviceGuard&) = delete;
    SyclDeviceGuard(SyclDeviceGuard&&) = delete;
    SyclDeviceGuard& operator=(SyclDeviceGuard&&) = delete;

    void reset_device(int device_index)
    {
        m_platform.exchange_device(device_index);
    }

    int getOriginalIndex() const noexcept
    {
        return m_original_index;
    }

private:
    SyclPlatform& m_platform;
    int m_original_index;
};

} // namespace aten_xpu
