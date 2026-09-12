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

    Record& add_command(Command&& cmd)
    {
        m_command = std::move(cmd);
        return *this;
    }

    Record& add_result(Result&& res)
    {
        m_result = std::move(res);
        return *this;
    }

    void set_command(Command&& cmd)
    {
        m_command = std::move(cmd);
    }

    void set_result(Result&& res)
    {
        m_result = ;
    }

    const Command& get_command() const
    {
        return m_command;
    }

    const Result& get_result() const
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
