module;

#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>

export module cc_utils_kernel:fd;

import std;

export namespace cc_utils::kernel {

class UniqueFd
{
public:
    explicit UniqueFd(int fd) :
        m_fd{fd}
    {
    }

    ~UniqueFd()
    {
        close();
    }

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    UniqueFd(UniqueFd&& other) noexcept :
        m_fd(std::exchange(other.m_fd, -1))
    {
    }

    UniqueFd& operator=(UniqueFd&& other) noexcept
    {
        if (this != &other) {
            close();
            m_fd = std::exchange(other.m_fd, -1);
        }
        return *this;
    }

    [[nodiscard]] static std::expected<UniqueFd, std::error_code> wrap(int raw_fd) noexcept
    {
        if (raw_fd < 0) {
            // Because the POSIX call was just made, errno is fresh and ready to capture.
            return std::unexpected(std::make_error_code(static_cast<std::errc>(errno)));
        }
        return UniqueFd{raw_fd};
    }

    UniqueFd& add_fd(int fd)
    {
        m_fd = fd;
        return *this;
    }

    // Relinquish ownership (useful when passing to another thread)
    [[nodiscard]] int release()
    {
        return std::exchange(m_fd, -1);
    }

    void reset(int fd)
    {
        close();
        m_fd = fd;
    }

    void close()
    {
        if (m_fd != -1) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    void set_fd(int fd)
    {
        m_fd = fd;
    }

    [[nodiscard]] const int& get_fd() const
    {
        return m_fd;
    }

private:
    int m_fd = -1;
};

} // namespace cc_utils::kernel
