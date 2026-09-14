module;

#include <cerrno>
#include <sys/wait.h>
#include <unistd.h>

export module cc_utils_cli:runner;

import std;
import cc_utils_pipe;
import :command;
import :result;
import :record;

export namespace cc_utils::cli {

class Runner
{
public:
    Runner() = default;

    ~Runner() = default;
    Runner(const Runner&) = delete;
    Runner& operator=(const Runner&) = delete;
    Runner(Runner&&) = default;
    Runner& operator=(Runner&&) = default;

    Runner& add_history(Record&& record) noexcept
    {
        m_history.emplace_back(std::move(record));
        return *this;
    }

    std::expected<Result, std::string> execute(Command&& cmd) noexcept
    {
        auto process_expected = pipe::Process::create(cmd.get_executable().get_name());
        if (!process_expected) {
            return std::unexpected{process_expected.error()};
        }
        auto& pipes = *process_expected;

        auto start_time = std::chrono::steady_clock::now();
        pid_t pid = ::fork();

        if (pid < 0) {
            return std::unexpected{"Failed to fork process for " + cmd.get_executable().get_name()};
        }

        if (pid == 0) {
            execute_child(cmd, pipes);
        }

        Result result = manage_parent_io(pid, cmd, pipes, start_time);

        m_history.emplace_back(Record{std::move(cmd), std::move(result)});

        return result;
    }

    double to_success_rate() const noexcept
    {
        if (m_history.empty()) {
            return 0.0;
        }

        double successes = std::ranges::count_if(
            m_history,
            [](const auto& record)
            {
                return record.is_success();
            }
        );

        return (successes / m_history.size()) * 100.0;
    }

    std::chrono::milliseconds to_mean_duration() const noexcept
    {
        if (m_history.empty()) {
            return std::chrono::milliseconds{0};
        }

        auto total = std::chrono::milliseconds{0};
        for (const auto& record: m_history) {
            total += record.get_duration();
        }

        return total / m_history.size();
    }

    void append_history(Record&& record) noexcept
    {
        m_history.emplace_back(std::move(record));
    }

    const std::span<const Record> get_history() const noexcept
    {
        return m_history;
    }


private:
    [[noreturn]] void execute_child(const Command& cmd, pipe::Process& pipes)
    {
        pipes.setup_child_redirects();

        auto c_args = cmd.to_c_args();
        const std::string& exe_str = cmd.get_executable().is_found()
                                         ? cmd.get_executable().get_path()
                                         : cmd.get_executable().get_name();

        ::execv(exe_str.c_str(), c_args.data());

        ::_exit(127);
    }

    [[nodiscard]] Result manage_parent_io(
        pid_t pid,
        const Command& cmd,
        pipe::Process& pipes,
        std::chrono::steady_clock::time_point start_time
    )
    {
        pipes.close_child_ends();

        std::jthread stdin_thread(
            [&pipes, &cmd]()
            {
                pipes.stream_stdin(cmd.get_stdin_text());
            }
        );

        std::string stderr_output;
        std::jthread stderr_thread(
            [&pipes, &stderr_output]()
            {
                stderr_output = pipes.read_stderr();
            }
        );

        std::string stdout_output = pipes.read_stdout();

        // Wait for child to exit
        int status = 0;
        ::waitpid(pid, &status, 0);

        auto end_time = std::chrono::steady_clock::now();

        int exit_code = -1;
        bool exited_normally = false;
        int term_signal = 0;

        if (WIFEXITED(status)) {
            exited_normally = true;
            exit_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            term_signal = WTERMSIG(status);
        }

        return Result{
            exit_code,
            exited_normally,
            term_signal,
            std::move(stdout_output),
            std::move(stderr_output),
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time)
        };
    }

    std::vector<Record> m_history;
};
} // namespace cc_utils::cli
