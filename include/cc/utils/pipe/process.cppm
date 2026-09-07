module;

#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>

export module cc_utils_pipe:process;

import std;
import :fd;
import :pipe;

export namespace cc_utils::pipe {

class Process
{
public:
    Process(Process&&) = default;
    Process& operator=(Process&&) = default;
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    // Factory method orchestrates the creation of all 3 required pipes
    static std::expected<Process, std::string> create(const std::string& exe_name)
    {
        auto stdin_pipe = Pipe::create();
        if (!stdin_pipe) {
            return std::unexpected{"Failed to create stdin pipe for " + exe_name};
        }

        auto stdout_pipe = Pipe::create();
        if (!stdout_pipe) {
            return std::unexpected{"Failed to create stdout pipe for " + exe_name};
        }

        auto stderr_pipe = Pipe::create();
        if (!stderr_pipe) {
            return std::unexpected{"Failed to create stderr pipe for " + exe_name};
        }

        return Process{std::move(*stdin_pipe), std::move(*stdout_pipe), std::move(*stderr_pipe)};
    }

    void stream_stdin(std::string_view input)
    {
        m_stdin.stream_write(input);
    }

    std::string read_stdout()
    {
        return m_stdout.read_all();
    }

    std::string read_stderr()
    {
        return m_stderr.read_all();
    }

    void setup_child_redirects()
    {
        m_stdin.close_write_end();
        m_stdout.close_read_end();
        m_stderr.close_read_end();

        ::dup2(m_stdin.get_read_end(), STDIN_FILENO);
        ::dup2(m_stdout.get_write_end(), STDOUT_FILENO);
        ::dup2(m_stderr.get_write_end(), STDERR_FILENO);

        m_stdin.close_read_end();
        m_stdout.close_write_end();
        m_stderr.close_write_end();
    }

    void close_child_ends()
    {
        m_stdin.close_read_end();
        m_stdout.close_write_end();
        m_stderr.close_write_end();
    }

    int release_parent_write_stdin()
    {
        return m_stdin.release_write_end();
    }

    int release_parent_read_stdout()
    {
        return m_stdout.release_read_end();
    }

    int release_parent_read_stderr()
    {
        return m_stderr.release_read_end();
    }

    int release_child_read_stdin()
    {
        return m_stdin.release_read_end();
    }

    int release_child_write_stdout()
    {
        return m_stdout.release_write_end();
    }

    int release_child_write_stderr()
    {
        return m_stderr.release_write_end();
    }

    int get_parent_write_stdin() const
    {
        return m_stdin.get_write_end();
    }

    int get_parent_read_stdout() const
    {
        return m_stdout.get_read_end();
    }

    int get_parent_read_stderr() const
    {
        return m_stderr.get_read_end();
    }

    int get_child_read_stdin() const
    {
        return m_stdin.get_read_end();
    }

    int get_child_write_stdout() const
    {
        return m_stdout.get_write_end();
    }

    int get_child_write_stderr() const
    {
        return m_stderr.get_write_end();
    }

private:
    Process(Pipe&& stdin_p, Pipe&& stdout_p, Pipe&& stderr_p) :
        m_stdin(std::move(stdin_p)),
        m_stdout(std::move(stdout_p)),
        m_stderr(std::move(stderr_p))
    {
    }

    Pipe m_stdin;
    Pipe m_stdout;
    Pipe m_stderr;
};

} // namespace cc_utils::pipe
