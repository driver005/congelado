module;

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:peer_access;

import std;

export namespace aten_xpu {

class SyclPeerAccess
{
public:
    void reset(std::size_t device_count)
    {
        m_device_count = device_count;
        m_cache.assign(device_count * device_count, -1);
        for (std::size_t index = 0; index < device_count; ++index) {
            m_cache[index * device_count + index] = 1;
        }
    }

    bool can_access(
        const sycl::device& device,
        std::size_t index,
        const sycl::device& peer,
        std::size_t peer_index
    )
    {
        auto& cached = m_cache.at(index * m_device_count + peer_index);
        if (cached == -1) {
            cached = static_cast<int8_t>(device.ext_oneapi_can_access_peer(
                peer,
                sycl::ext::oneapi::peer_access::access_supported
            ));
        }
        return cached == 1;
    }

    bool enable(
        const sycl::device& device,
        std::size_t index,
        const sycl::device& peer,
        std::size_t peer_index
    )
    {
        if (!can_access(device, index, peer, peer_index)) {
            return false;
        }

        if (index != peer_index) {
            device.ext_oneapi_enable_peer_access(peer);
        }
        return true;
    }

private:
    std::size_t m_device_count{0};
    std::vector<int8_t> m_cache;
};

} // namespace aten_xpu
