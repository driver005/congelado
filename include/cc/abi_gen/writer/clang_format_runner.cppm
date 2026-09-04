module;

#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

export module cc_abi_gen_writer:clang_format_runner;

import std;

export namespace cc_abi_gen {

// Pipes generated source text through the repo's own `clang-format -style=file`, since
// .clang-format is this project's single source of truth for whitespace/brace/wrapping — the
// emitters only need to produce syntactically valid, semantically correct C++ text.
class ClangFormatRunner
{
public:
    std::expected<std::string, std::string> format(
        const std::string &source, const std::filesystem::path &repo_root)
    {
        std::filesystem::path temp_path = temp_file_path();

        std::ofstream temp_file(temp_path);
        if (!temp_file) {

            return std::unexpected{"failed to create temp file for clang-format"};
        }
        temp_file << source;
        temp_file.close();

        std::filesystem::path style_path = repo_root / ".clang-format";
        m_scratch_command = "clang-format -style=file:" + style_path.string() + " " +
            temp_path.string();

        auto result = run_command(m_scratch_command);
        std::filesystem::remove(temp_path);

        if (!result) {

            return std::unexpected{result.error()};
        }

        return std::move(*result);
    }

private:
    // Distinct genrule invocations are separate OS processes running concurrently (Bazel
    // parallelizes independent genrules), so pid + a per-instance counter (this instance lives
    // for exactly one process's run) makes the path unique across them.
    std::filesystem::path temp_file_path()
    {
        return std::filesystem::temp_directory_path() /
            ("cc_abi_gen_" + std::to_string(getpid()) + "_" + std::to_string(m_counter++) + ".cppm");
    }

    std::expected<std::string, std::string> run_command(const std::string &command)
    {
        std::FILE *pipe = popen(command.c_str(), "r");
        if (pipe == nullptr) {

            return std::unexpected{"failed to launch: " + command};
        }

        m_scratch_output.clear();
        std::array<char, 4096> buffer{};
        std::size_t read_count = 0;
        while ((read_count = std::fread(buffer.data(), 1, buffer.size(), pipe)) > 0) {

            m_scratch_output.append(buffer.data(), read_count);
        }

        int wait_status = pclose(pipe);
        int exit_status = WIFEXITED(wait_status) ? WEXITSTATUS(wait_status) : -1;
        if (exit_status != 0) {

            return std::unexpected{command + " exited with status " + std::to_string(exit_status)};
        }

        return m_scratch_output;
    }

    unsigned m_counter = 0;
    std::string m_scratch_command;
    std::string m_scratch_output;
};

} // namespace cc_abi_gen
