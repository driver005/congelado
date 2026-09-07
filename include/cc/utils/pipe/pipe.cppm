export module cc_utils_pipe:pipe;

import std;
import <cstdio>;
export import :fd;
export import :process;

export namespace cc_utils::pipe {

class Pipe
{
public:
    Pipe(Pipe&&) = default;
    Pipe& operator=(Pipe&&) = default;
    Pipe(const Pipe&) = delete;
    Pipe& operator=(const Pipe&) = delete;

    static std::expected<Pipe, std::string> create()
    {
        int fds[2];
        if (::pipe(fds) < 0) {
            return std::unexpected{"Failed to create POSIX pipe"};
        }
        return Pipe{fds};
    }

    void stream_write(std::string_view input)
    {
        int fd = m_write_end.get();
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

    std::string read_all()
    {
        std::string output;
        int fd = m_read_end.get();
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

    int get_read_end() const
    {
        return m_read_end.get();
    }

    int get_write_end() const
    {
        return m_write_end.get();
    }

    int release_read_end()
    {
        return m_read_end.release();
    }

    int release_write_end()
    {
        return m_write_end.release();
    }

    void close_read_end()
    {
        m_read_end.close();
    }

    void close_write_end()
    {
        m_write_end.close();
    }

private:
    explicit Pipe(int fds[2]) :
        m_read_end(fds[0]),
        m_write_end(fds[1])
    {
    }

    UniqueFd m_read_end;
    UniqueFd m_write_end;
};
