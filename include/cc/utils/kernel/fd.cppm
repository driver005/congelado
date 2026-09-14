module;

#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>

export module cc_utils_kernel:fd;

import std;

export namespace cc_utils::kernel {

class FileDescriptor
{
public:
    explicit FileDescriptor(int fd) :
        m_fd{fd}
    {
    }

    [[nodiscard]] static std::expected<FileDescriptor, std::error_code> wrap(int raw_fd) noexcept
    {
        if (raw_fd < 0) {
            // Because the POSIX call was just made, errno is fresh and ready to capture.
            return std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));
        }
        return FileDescriptor{raw_fd};
    }

    ~FileDescriptor()
    {
        close();
    }

    FileDescriptor(const FileDescriptor&) = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other) noexcept :
        m_fd(std::exchange(other.m_fd, -1))
    {
    }

    FileDescriptor& operator=(FileDescriptor&& other) noexcept
    {
        if (this != &other) {
            close();
            m_fd = std::exchange(other.m_fd, -1);
        }
        return *this;
    }

    FileDescriptor& add_fd(int fd) noexcept
    {
        m_fd = fd;
        return *this;
    }

    // Relinquish ownership (useful when passing to another thread)
    [[nodiscard]] int release() noexcept
    {
        return std::exchange(m_fd, -1);
    }

    void reset(int fd) noexcept
    {
        close();
        m_fd = fd;
    }

    void close() noexcept
    {
        if (m_fd != -1) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    void set_fd(int fd) noexcept
    {
        m_fd = fd;
    }

    [[nodiscard]] int get_fd() noexcept
    {
        return m_fd;
    }

    [[nodiscard]] const int get_fd() const noexcept
    {
        return m_fd;
    }

private:
    int m_fd = -1;
};

} // namespace cc_utils::kernel
