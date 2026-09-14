module;

#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>

export module cc_utils_pipe:process;

import std;
import :pipe;

export namespace cc_utils::pipe {

class Process
{
public:
    Process(Pipe&& stdin_p, Pipe&& stdout_p, Pipe&& stderr_p) :
        m_stdin(std::move(stdin_p)),
        m_stdout(std::move(stdout_p)),
        m_stderr(std::move(stderr_p))
    {
    }

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&&) = default;
    Process& operator=(Process&&) = default;

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

    Process& add_stdin(Pipe&& stdin_pipe)
    {
        m_stdin = std::move(stdin_pipe);
        return *this;
    }

    Process& add_stdout(Pipe&& stdout_pipe)
    {
        m_stdout = std::move(stdout_pipe);
        return *this;
    }

    Process& add_stderr(Pipe&& stderr_pipe)
    {
        m_stderr = std::move(stderr_pipe);
        return *this;
    }

    void stream_stdin(std::string_view input)
    {
        m_stdin.stream_write(input);
    }

    [[nodiscard]] std::string read_stdout()
    {
        return m_stdout.read_all();
    }

    [[nodiscard]] std::string read_stderr()
    {
        return m_stderr.read_all();
    }

    void setup_redirects()
    {
        m_stdin.setup_redirect<false>();
        m_stdout.setup_redirect<true>();
        m_stderr.setup_redirect<true>();
    }

    void close_ends()
    {
        m_stdin.close();
        m_stdout.close();
        m_stderr.close();
    }

    void set_stdin(Pipe&& stdin_pipe)
    {
        m_stdin = std::move(stdin_pipe);
    }

    void set_stdout(Pipe&& stdout_pipe)
    {
        m_stdout = std::move(stdout_pipe);
    }

    void set_stderr(Pipe&& stderr_pipe)
    {
        m_stderr = std::move(stderr_pipe);
    }

    const Pipe& stdin_pipe() const noexcept
    {
        return m_stdin;
    }

    const Pipe& stdout_pipe() const noexcept
    {
        return m_stdout;
    }

    const Pipe& stderr_pipe() const noexcept
    {
        return m_stderr;
    }


private:
    Pipe m_stdin;
    Pipe m_stdout;
    Pipe m_stderr;
};

} // namespace cc_utils::pipe
