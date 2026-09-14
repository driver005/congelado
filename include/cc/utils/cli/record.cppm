export module cc_utils_cli:record;

import std;
import :command;
import :result;

export namespace cc_utils::cli {

class Record
{
public:
    Record(Command&& cmd, Result&& res) :
        m_command{std::move(cmd)},
        m_result{std::move(res)}
    {
    }

    ~Record() = default;

    Record(const Record&) = delete;
    Record& operator=(const Record&) = delete;
    Record(Record&&) = default;
    Record& operator=(Record&&) = default;

    Record& add_command(Command&& cmd) noexcept
    {
        m_command = std::move(cmd);
        return *this;
    }

    Record& add_result(Result&& res) noexcept
    {
        m_result = std::move(res);
        return *this;
    }

    bool is_success() const noexcept
    {
        return m_result.success();
    }

    // Generates a quick human-readable log string (e.g., "clang-format [SUCCESS] - 45ms")
    std::string to_summary() const noexcept
    {
        return m_command.get_executable().get_name() + (is_success() ? " [SUCCESS]" : " [FAILED]") +
               " - " + std::to_string(get_duration().count()) + "ms";
    }

    void set_command(Command&& cmd) noexcept
    {
        m_command = std::move(cmd);
    }

    void set_result(Result&& res) noexcept
    {
        m_result = std::move(res);
    }

    const Command& get_command() const noexcept
    {
        return m_command;
    }

    const Result& get_result() const noexcept
    {
        return m_result;
    }


private:
    Command m_command;
    Result m_result;
};

} // namespace cc_utils::cli
