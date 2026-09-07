export module cc_utils_cli:record;

import std;
import :command;
import :result;

export namespace cc_utils::cli {

class Record
{
public:
    Record(const Command&& cmd, const CommandResult&& res) :
        m_command{std::move(cmd)},
        m_result{std::move(res)}
    {
    }

    // Read-only accessors
    const Command& get_command() const
    {
        return m_command;
    }

    const CommandResult& get_result() const
    {
        return m_result;
    }

    bool is_success() const
    {
        return m_result.success();
    }

    std::chrono::milliseconds get_duration() const
    {
        return m_result.get_duration();
    }

    // Generates a quick human-readable log string (e.g., "clang-format [SUCCESS] - 45ms")
    std::string get_summary() const
    {
        return m_command.get_executable().get_name() + (is_success() ? " [SUCCESS]" : " [FAILED]") +
               " - " + std::to_string(get_duration().count()) + "ms";
    }

private:
    Command m_command;
    Result m_result;
};

} // namespace cc_utils::cli
