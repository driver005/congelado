module;

#include "include/c/extern/filesystem/writable_file.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

export module aten_xpu_extern_filesystem:writable_file;

import std;
import cc_ice_extern_filesystem_builder;
import cc_ice_intern_sonic;
import aten_xpu_intern;

export namespace aten_xpu {

// A file opened for appending writes, with no user space buffering: flush has nothing to do and
// sync calls fsync.
class WritableFile : public ice::builder::WritableFile
{
public:
    WritableFile(
        const ::TF_StatusOps* status_ops,
        const ::TF_StringOps* string_ops,
        const std::string& path,
        bool truncate
    ) noexcept :
        ice::builder::WritableFile{status_ops, string_ops},
        m_failure{string_ops},
        m_path{path},
        m_descriptor{::open(
            path.c_str(),
            O_WRONLY | O_CREAT | O_CLOEXEC | (truncate ? O_TRUNC : O_APPEND),
            0644
        )},
        m_open_error{m_descriptor < 0 ? errno : 0}
    {
    }

    ~WritableFile() override
    {
        if (m_descriptor >= 0) {
            ::close(m_descriptor);
        }
    }

    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;
    WritableFile(WritableFile&&) = delete;
    WritableFile& operator=(WritableFile&&) = delete;

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

    void append(
        const ice::sonic::String& buffer,
        const ice::sonic::Status& out_status
    ) noexcept override
    {
        if (!ensure_open(out_status)) {
            return;
        }

        const std::string_view text = String::view_of(buffer);
        std::size_t written = 0;
        while (written < text.size()) {
            const ssize_t count =
                ::write(m_descriptor, text.data() + written, text.size() - written);
            if (count < 0) {
                m_failure.fail_errno(out_status, errno, m_path);
                return;
            }
            written += static_cast<std::size_t>(count);
        }
    }

    void tell(std::int64_t& out_position, const ice::sonic::Status& out_status) noexcept override
    {
        out_position = 0;
        if (!ensure_open(out_status)) {
            return;
        }

        const off_t position = ::lseek(m_descriptor, 0, SEEK_CUR);
        if (position < 0) {
            m_failure.fail_errno(out_status, errno, m_path);
            return;
        }

        out_position = static_cast<std::int64_t>(position);
    }

    void flush(const ice::sonic::Status& out_status) noexcept override
    {
        ensure_open(out_status);
    }

    void sync(const ice::sonic::Status& out_status) noexcept override
    {
        if (ensure_open(out_status) && ::fsync(m_descriptor) != 0) {
            m_failure.fail_errno(out_status, errno, m_path);
        }
    }

    void close(const ice::sonic::Status& out_status) noexcept override
    {
        if (m_descriptor < 0) {
            return;
        }

        const int result = ::close(m_descriptor);
        m_descriptor = -1;
        m_open_error = EBADF;
        if (result != 0) {
            m_failure.fail_errno(out_status, errno, m_path);
        }
    }

private:
    bool ensure_open(const ice::sonic::Status& out_status) const noexcept
    {
        if (m_descriptor >= 0) {
            return true;
        }

        m_failure.fail_errno(out_status, m_open_error, m_path);

        return false;
    }

    Failure m_failure;
    std::string m_path;
    int m_descriptor;
    int m_open_error;
};

} // namespace aten_xpu
