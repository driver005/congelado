module;

#include "include/c/extern/filesystem/random_access_file.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

export module aten_xpu_extern_filesystem:random_access_file;

import std;
import cc_ice_extern_filesystem_builder;
import cc_ice_intern_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// A file opened for positional reads. A short read is an out of range error with the bytes read.
class RandomAccessFile : public ice::builder::RandomAccessFile
{
public:
    RandomAccessFile(
        const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        const std::string& path
    ) noexcept :
        ice::builder::RandomAccessFile{status_ops, string_ops},
        m_failure{string_ops},
        m_path{path},
        m_descriptor{::open(path.c_str(), O_RDONLY | O_CLOEXEC)},
        m_open_error{m_descriptor < 0 ? errno : 0}
    {
    }

    ~RandomAccessFile() override
    {
        if (m_descriptor >= 0) {
            ::close(m_descriptor);
        }
    }

    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;
    RandomAccessFile(RandomAccessFile&&) = delete;
    RandomAccessFile& operator=(RandomAccessFile&&) = delete;

    // 0 when the file opened, otherwise the errno of the failed open.
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

    void read(
        std::uint64_t offset,
        std::size_t n,
        char* buffer,
        std::int64_t& out_bytes_read,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        out_bytes_read = 0;
        if (m_descriptor < 0) {
            m_failure.fail_errno(out_status, m_open_error, m_path);
            return;
        }

        std::size_t total = 0;
        while (total < n) {
            const ssize_t count = ::pread(
                m_descriptor,
                buffer + total,
                n - total,
                static_cast<off_t>(offset + total)
            );
            if (count < 0) {
                out_bytes_read = static_cast<std::int64_t>(total);
                m_failure.fail_errno(out_status, errno, m_path);
                return;
            }
            if (count == 0) {
                break;
            }
            total += static_cast<std::size_t>(count);
        }

        out_bytes_read = static_cast<std::int64_t>(total);
        if (total < n) {
            m_failure.fail(
                out_status,
                ice::sonic::Code::k_out_of_range,
                "read past the end of " + m_path
            );
        }
    }

private:
    Failure m_failure;
    std::string m_path;
    int m_descriptor;
    int m_open_error;
};

} // namespace aten_xpu
