module;

#include "include/c/extern/stream_executor/types.h"

#include <sycl/sycl.hpp>

export module aten_xpu_extern_stream_executor:allocator_ipc_memory;

import std;

export namespace aten_xpu {

class SyclIpcMemory
{
public:
    static constexpr std::uint8_t k_handle_version = 1;
    static constexpr std::uint8_t k_handle_type_device_malloc = 0;
    static constexpr std::size_t k_header_size = 3 + sizeof(std::int64_t);

    SyclIpcMemory(const sycl::context& context, const sycl::device& device) :
        m_context{context},
        m_device{device}
    {
    }

    ~SyclIpcMemory() { close_all(); }

    SyclIpcMemory(const SyclIpcMemory&) = delete;
    SyclIpcMemory& operator=(const SyclIpcMemory&) = delete;
    SyclIpcMemory(SyclIpcMemory&&) = delete;
    SyclIpcMemory& operator=(SyclIpcMemory&&) = delete;

    void export_handle(
        void* segment_base,
        std::int64_t offset,
        std::uint64_t allocation_size,
        TF_IpcMemoryHandle& out_handle
    ) const
    {

        namespace ipc_memory = sycl::ext::oneapi::experimental::ipc_memory;

        const auto handle_data = ipc_memory::get(segment_base, m_context).data();
        if (handle_data.size() + k_header_size > sizeof(out_handle.data)) {
            throw std::length_error{"IPC memory handle exceeds 128 bytes"};
        }

        out_handle = TF_IpcMemoryHandle{.struct_size = sizeof(TF_IpcMemoryHandle)};
        out_handle.data[0] = k_handle_version;
        out_handle.data[1] = k_handle_type_device_malloc;
        out_handle.data[2] = static_cast<std::uint8_t>(handle_data.size());
        std::memcpy(out_handle.data + 3, &offset, sizeof(offset));
        std::memcpy(out_handle.data + k_header_size, handle_data.data(), handle_data.size());
        out_handle.data_size = k_header_size + handle_data.size();
        out_handle.allocation_size = allocation_size;

    }

    void* open(const TF_IpcMemoryHandle& handle)
    {

        namespace ipc_memory = sycl::ext::oneapi::experimental::ipc_memory;

        if (handle.data[0] > k_handle_version || handle.data[1] != k_handle_type_device_malloc) {
            throw std::invalid_argument{"unknown IPC memory handle format"};
        }

        const std::string key(
            reinterpret_cast<const char*>(handle.data),
            static_cast<std::size_t>(handle.data_size)
        );
        auto found = m_opened.find(key);
        if (found == m_opened.end()) {
            const auto handle_size = static_cast<std::size_t>(handle.data[2]);
            const auto* bytes = reinterpret_cast<const std::byte*>(handle.data + k_header_size);
            const ipc_memory::handle_data_t handle_data(bytes, bytes + handle_size);
            void* base = ipc_memory::open(handle_data, m_context, m_device);
            found = m_opened.emplace(key, std::pair{base, 0}).first;
        }
        ++found->second.second;

        std::int64_t offset = 0;
        std::memcpy(&offset, handle.data + 3, sizeof(offset));
        auto* pointer = static_cast<std::byte*>(found->second.first) + offset;
        m_pointer_keys.emplace(pointer, key);
        return pointer;

    }

    bool close(void* pointer)
    {

        const auto key_entry = m_pointer_keys.find(pointer);
        if (key_entry == m_pointer_keys.end()) {
            return false;
        }

        auto found = m_opened.find(key_entry->second);
        m_pointer_keys.erase(key_entry);
        if (found != m_opened.end() && --found->second.second == 0) {
            sycl::ext::oneapi::experimental::ipc_memory::close(found->second.first, m_context);
            m_opened.erase(found);
        }
        return true;

    }

private:
    void close_all() noexcept
    {

        for (auto& [key, entry]: m_opened) {
            try {
                sycl::ext::oneapi::experimental::ipc_memory::close(entry.first, m_context);
            } catch (const sycl::exception&) {
            }
        }
        m_opened.clear();
        m_pointer_keys.clear();

    }

    sycl::context m_context;
    sycl::device m_device;
    std::unordered_map<std::string, std::pair<void*, int>> m_opened;
    std::unordered_multimap<void*, std::string> m_pointer_keys;
};

} // namespace aten_xpu
