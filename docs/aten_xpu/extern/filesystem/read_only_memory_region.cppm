module;

#include "include/c/extern/filesystem/read_only_memory_region.h"
#include "include/c/intern/tstring.h"

#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

export module aten_xpu_extern_filesystem:read_only_memory_region;

import std;
import cc_ice_extern_filesystem_builder;
import cc_ice_intern_sonic;

export namespace aten_xpu {

// A file mapped read only; an empty file maps to a null pointer and length 0.
class ReadOnlyMemoryRegion : public ice::builder::ReadOnlyMemoryRegion
{
public:
    ReadOnlyMemoryRegion(const ::TF_StringOps* string_ops, const std::string& path) noexcept :
        ice::builder::ReadOnlyMemoryRegion{string_ops},
        m_path{path}
    {
        const int descriptor = ::open(path.c_str(), O_RDONLY | O_CLOEXEC);
        if (descriptor < 0) {
            m_open_error = errno;
            return;
        }

        struct stat info{};
        if (::fstat(descriptor, &info) != 0) {
            m_open_error = errno;
        } else if (info.st_size > 0) {
            void* mapping = ::mmap(
                nullptr,
                static_cast<std::size_t>(info.st_size),
                PROT_READ,
                MAP_PRIVATE,
                descriptor,
                0
            );
            if (mapping == MAP_FAILED) {
                m_open_error = errno;
            } else {
                m_data = mapping;
                m_length = static_cast<std::uint64_t>(info.st_size);
            }
        }
        ::close(descriptor);
    }

    ~ReadOnlyMemoryRegion() override
    {
        if (m_data != nullptr) {
            ::munmap(m_data, static_cast<std::size_t>(m_length));
        }
    }

    ReadOnlyMemoryRegion(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion& operator=(const ReadOnlyMemoryRegion&) = delete;
    ReadOnlyMemoryRegion(ReadOnlyMemoryRegion&&) = delete;
    ReadOnlyMemoryRegion& operator=(ReadOnlyMemoryRegion&&) = delete;

    // 0 when the file mapped (or is empty), otherwise the errno of the failure.
    int getOpenError() const noexcept
    {
        return m_open_error;
    }

    void destroy() noexcept override
    {
        delete this;
    }

    void get_name(const ice::sonic::String& out_name) noexcept override
    {
        out_name.copy(m_path);
    }

    void data(const void** out_data) noexcept override
    {
        *out_data = m_data;
    }

    void length(std::uint64_t& out_length) noexcept override
    {
        out_length = m_length;
    }

private:
    std::string m_path;
    void* m_data = nullptr;
    std::uint64_t m_length = 0;
    int m_open_error = 0;
};

} // namespace aten_xpu
