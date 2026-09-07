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

    std::expected<Result, std::string> execute(const Command& cmd)
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

        m_history.emplace_back(cmd, result);

        return result;
    }

    std::span<const Record> get_history() const
    {
        return m_history;
    }

    double get_success_rate() const
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

    std::chrono::milliseconds get_mean_duration() const
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

    Result manage_parent_io(
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
