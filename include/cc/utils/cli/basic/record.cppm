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

    ~Record() = default;
    Record(const Record&) = dele;
    Record(Record&&) = default;
    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) = default;

    Record& set_command(Command&& command) noexcept
    {
        m_command = std::move(command);
        return *this;
    }

    Record& set_result(Result&& result) noexcept
    {
        m_result = std::move(result);
        return *this;
    }

    [[nodiscard]] bool is_success() const noexcept
    {
        return m_result.success();
    }

    // Generates a quick human-readable log string (e.g., "clang-format [SUCCESS] - 45ms")
    [[nodiscard]] std::string to_summary() const
    {
        return m_command.get_executable().get_name() + (is_success() ? " [SUCCESS]" : " [FAILED]") +
               " - " + std::to_string(get_duration().count()) + "ms";
    }

    void set_command(Command&& command) noexcept
    {
        m_command = std::move(command);
    }

    void set_result(Result&& result) noexcept
    {
        m_result = std::move(result);
    }

    [[nodiscard]] const Command& get_command() const noexcept
    {
        return m_command;
    }

    [[nodiscard]] const Result& get_result() const noexcept
    {
        return m_result;
    }

    [[nodiscard]] const std::chrono::milliseconds& get_duration() const noexcept
    {
        return m_result.get_duration();
    }


private:
    Command m_command;
    Result m_result;
};

} // namespace cc_utils::cli
