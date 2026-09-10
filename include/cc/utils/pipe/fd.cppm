module;

#include <sys/wait.h>
#include <unistd.h>

export module cc_utils_pipe:fd;

import std;

export namespace cc_utils::pipe {

class UniqueFd
{
public:
    UniqueFd() = default;

    explicit UniqueFd(int fd) :
        m_fd(fd)
    {
    }

    ~UniqueFd()
    {
        close();
    }

    // Move-only semantics
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

    [[nodiscard]] int get() const
    {
        return m_fd;
    }

    // Relinquish ownership (useful when passing to another thread)
    int release()
    {
        return std::exchange(m_fd, -1);
    }

    void close()
    {
        if (m_fd != -1) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

private:
    int m_fd = -1;
};

} // namespace cc_utils::pipe
