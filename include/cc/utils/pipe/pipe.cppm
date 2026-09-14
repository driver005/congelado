module;

#include <cerrno>
#include <unistd.h>

export module cc_utils_pipe:pipe;

import std;
import cc_utils_kernel;

export namespace cc_utils::pipe {

class Pipe
{
public:
    explicit Pipe(int fds[2]) :
        m_read_end(fds[0]),
        m_write_end(fds[1])
    {
    }

    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;
    Pipe(Pipe&&) = default;
    Pipe& operator=(Pipe&&) = default;

    Pipe& add_read_end(kernel::FileDescriptor&& fd) noexcept
    {
        m_read_end = std::move(fd);
        return *this;
    }

    Pipe& add_write_end(kernel::FileDescriptor&& fd) noexcept
    {
        m_write_end = std::move(fd);
        return *this;
    }

    static std::expected<Pipe, std::string> create() noexcept
    {
        int fds[2];
        if (::pipe(fds) < 0) {
            return std::unexpected{"Failed to create POSIX pipe"};
        }
        return Pipe{fds};
    }

    template<bool IsReader>
    void setup_redirect() noexcept
    {
        if constexpr (IsReader) {
            m_read_end.close();
        } else {
            m_write_end.close();
        }

        if constexpr (IsReader) {
            ::dup2(m_write_end.get_fd(), STDOUT_FILENO);
        } else {
            ::dup2(m_read_end.get_fd(), STDIN_FILENO);
        }

        if constexpr (IsReader) {
            m_write_end.close();
        } else {
            m_read_end.close();
        }
    }

    void stream_write(std::string_view input) noexcept
    {
        int fd = m_write_end.get_fd();
        if (fd == -1) {
            return;
        }

        const char* data = input.data();
        std::size_t remaining = input.size();
        while (remaining > 0) {
            ssize_t written = ::write(fd, data, remaining);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            data += written;
            remaining -= written;
        }
        // Send EOF immediately
        m_write_end.close();
    }

    std::string read_all() noexcept
    {
        std::string output;
        int fd = m_read_end.get_fd();
        if (fd == -1) {
            return output;
        }

        std::array<char, 4'096> buffer{};
        while (true) {
            ssize_t bytes_read = ::read(fd, buffer.data(), buffer.size());
            if (bytes_read < 0) {
                if (errno == EINTR) {
                    continue;
                }
                break;
            }
            // EOF reached
            if (bytes_read == 0) {
                break;
            }
            output.append(buffer.data(), bytes_read);
        }
        return output;
    }

    void close() noexcept
    {
        m_read_end.close();
        m_write_end.close();
    }

    void set_read_end(kernel::FileDescriptor&& fd) noexcept
    {
        m_read_end = std::move(fd);
    }

    void set_write_end(kernel::FileDescriptor&& fd) noexcept
    {
        m_write_end = std::move(fd);
    }

    const kernel::FileDescriptor& get_read_end() const noexcept
    {
        return m_read_end;
    }

    const kernel::FileDescriptor& get_write_end() const noexcept
    {
        return m_write_end;
    }

private:
    kernel::FileDescriptor m_read_end;
    kernel::FileDescriptor m_write_end;
};

} // namespace cc_utils::pipe
