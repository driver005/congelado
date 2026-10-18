// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/stream_executor/types.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/stream_executor/types.h"

export module cc_ice_extern_stream_executor_sonic:ipc_memory_handle;

import std;

export namespace ice::sonic {

class IpcMemoryHandle
{
public:
    IpcMemoryHandle() noexcept
    {
        m_value.struct_size = sizeof(::TF_IpcMemoryHandle);
    }

    explicit IpcMemoryHandle(const ::TF_IpcMemoryHandle& value) noexcept :
        m_value{value}
    {
    }

    std::span<std::uint8_t, 128> getData() noexcept
    {
        return m_value.data;
    }

    std::span<const std::uint8_t, 128> getData() const noexcept
    {
        return m_value.data;
    }

    std::uint64_t getDataSize() const noexcept
    {
        return m_value.data_size;
    }

    void setDataSize(std::uint64_t value) noexcept
    {
        m_value.data_size = value;
    }

    std::uint64_t getAllocationSize() const noexcept
    {
        return m_value.allocation_size;
    }

    void setAllocationSize(std::uint64_t value) noexcept
    {
        m_value.allocation_size = value;
    }

    ::TF_IpcMemoryHandle* get_handle() noexcept
    {
        return &m_value;
    }

    const ::TF_IpcMemoryHandle* get_handle() const noexcept
    {
        return &m_value;
    }

private:
    ::TF_IpcMemoryHandle m_value{};
};

} // namespace ice::sonic
