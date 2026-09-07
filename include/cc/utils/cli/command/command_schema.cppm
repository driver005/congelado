export module cc_utils_cli:command_schema;

import std;

export namespace cc_utils::cli {

// One command a Parser accepts (e.g. "generate") and the `--flag` names it allows.
class CommandSchema
{
public:
    CommandSchema(std::string&& name, std::vector<std::string>&& flags) :
        m_name{std::move(name)},
        m_flags{std::move(flags)}
    {
    }

    const std::string& get_name() const
    {
        return m_name;
    }

    bool accepts_flag(const std::string& flag) const
    {
        return std::ranges::find(m_flags, flag) != m_flags.end();
    }

    const std::vector<std::string>& get_flags() const
    {
        return m_flags;
    }

private:
    std::string m_name;
    std::vector<std::string> m_flags;
};

} // namespace cc_utils::cli
