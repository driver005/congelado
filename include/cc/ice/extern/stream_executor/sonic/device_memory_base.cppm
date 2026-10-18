// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/types.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/types.h"

export module cc_ice_extern_stream_executor_sonic:device_memory_base;

import std;

export namespace ice::sonic {

class DeviceMemoryBase
{
public:
    DeviceMemoryBase() noexcept
    {
        m_value.struct_size = sizeof(::TF_DeviceMemoryBase);
    }

    explicit DeviceMemoryBase(const ::TF_DeviceMemoryBase& value) noexcept :
        m_value{value}
    {
    }

    DeviceMemoryBase(void* opaque, std::uint64_t size, std::uint64_t payload) noexcept
    {
        m_value.struct_size = sizeof(::TF_DeviceMemoryBase);
        m_value.opaque = opaque;
        m_value.size = size;
        m_value.payload = payload;
    }

    void* getOpaque() const noexcept
    {
        return m_value.opaque;
    }

    void setOpaque(void* value) noexcept
    {
        m_value.opaque = value;
    }

    std::uint64_t getSize() const noexcept
    {
        return m_value.size;
    }

    void setSize(std::uint64_t value) noexcept
    {
        m_value.size = value;
    }

    std::uint64_t getPayload() const noexcept
    {
        return m_value.payload;
    }

    void setPayload(std::uint64_t value) noexcept
    {
        m_value.payload = value;
    }

    ::TF_DeviceMemoryBase* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_DeviceMemoryBase* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_DeviceMemoryBase m_value{};
};

} // namespace ice::sonic
