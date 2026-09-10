export module cc_utils_cli:basic_record;

import std;
import :basic_command;
import :basic_result;

export namespace cc_utils::cli {

class Record
{
public:
    Record(Command&& cmd, Result&& res) noexcept :
        m_command{std::move(cmd)},
        m_result{std::move(res)}
    {
    }

    void set_command(Command&& command) noexcept
    {
        m_command = std::move(command);
    }

    void set_result(Result&& result) noexcept
    {
        m_result = std::move(result);
    }

    [[nodiscard]] Command& get_command() noexcept
    {
        return m_command;
    }

    [[nodiscard]] const Command& get_command() const noexcept
    {
        return m_command;
    }

    [[nodiscard]] Result& get_result() noexcept
    {
        return m_result;
    }

    [[nodiscard]] const Result& get_result() const noexcept
    {
        return m_result;
    }

    [[nodiscard]] bool is_success() const noexcept
    {
        return m_result.success();
    }

    [[nodiscard]] std::chrono::milliseconds get_duration() const noexcept
    {
        return m_result.get_duration();
    }

    // Generates a quick human-readable log string (e.g., "clang-format [SUCCESS] - 45ms")
    [[nodiscard]] std::string get_summary() const
    {
        return m_command.get_executable().get_name() + (is_success() ? " [SUCCESS]" : " [FAILED]") +
               " - " + std::to_string(get_duration().count()) + "ms";
    }

private:
    Command m_command;
    Result m_result;
};

} // namespace cc_utils::cli
