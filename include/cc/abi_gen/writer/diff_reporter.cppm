module;

#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

export module cc_abi_gen_writer:diff_reporter;

import std;
import :diff_result;

export namespace cc_abi_gen {

// Dry-run comparison for the `check` subcommand: shells out to `diff -u` between the real
// checked-in file and freshly generated text (written to a temp file first). Writes nothing.
class DiffReporter
{
public:
    std::expected<DiffResult, std::string> compare(
        const std::filesystem::path &real_path, const std::string &generated_text)
    {
        std::filesystem::path temp_path = temp_file_path();

        std::ofstream temp_file(temp_path);
        if (!temp_file) {

            return std::unexpected{"failed to create temp file for diff"};
        }
        temp_file << generated_text;
        temp_file.close();

        m_scratch_command = "diff -u " + real_path.string() + " " + temp_path.string();

        std::FILE *pipe = popen(m_scratch_command.c_str(), "r");
        if (pipe == nullptr) {

            std::filesystem::remove(temp_path);
            return std::unexpected{"failed to launch: " + m_scratch_command};
        }

        m_scratch_output.clear();
        std::array<char, 4096> buffer{};
        std::size_t read_count = 0;
        while ((read_count = std::fread(buffer.data(), 1, buffer.size(), pipe)) > 0) {

            m_scratch_output.append(buffer.data(), read_count);
        }

        int wait_status = pclose(pipe);
        std::filesystem::remove(temp_path);

        // pclose() returns the raw wait() status word, not a plain exit code — WEXITSTATUS
        // extracts it. `diff` exits 0 for identical input, 1 for differing input, and 2+ on a
        // real error (missing file, bad usage) — only >1 is treated as failure here.
        int exit_status = WIFEXITED(wait_status) ? WEXITSTATUS(wait_status) : -1;
        if (exit_status > 1) {

            return std::unexpected{
                m_scratch_command + " exited with status " + std::to_string(exit_status)};
        }

        DiffResult result;
        result.m_identical = (exit_status == 0);
        result.m_unified_diff = m_scratch_output;

        return result;
    }

private:
    // Distinct genrule invocations are separate OS processes running concurrently, so pid + a
    // per-instance counter (this instance lives for exactly one process's run) makes the path
    // unique across them.
    std::filesystem::path temp_file_path()
    {
        return std::filesystem::temp_directory_path() /
            ("cc_abi_gen_diff_" + std::to_string(getpid()) + "_" + std::to_string(m_counter++) +
                ".cppm");
    }

    unsigned m_counter = 0;
    std::string m_scratch_command;
    std::string m_scratch_output;
};

} // namespace cc_abi_gen
