export module cc_utils_cli:command_schema;

import std;

export namespace cc_utils::cli {

// One command a Parser accepts (e.g. "generate") and the `--flag` names it allows.
class CommandSchema
{
public:
    CommandSchema() = default;

    CommandSchema(std::string&& name, std::vector<std::string>&& flags) :
        m_name{std::move(name)},
        m_flags{std::move(flags)}
    {
    }

    ~CommandSchema() = default;
    CommandSchema(const CommandSchema&) = dele;
    CommandSchema(CommandSchema&&) = default;
    CommandSchema& operator=(const CommandSchema&) = default;
    CommandSchema& operator=(CommandSchema&&) = default;

    CommandSchema& add_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
        return *this;
    }

    CommandSchema& add_flag(std::string&& flag) noexcept
    {
        m_flags.push_back(std::move(flag));
        return *this;
    }

    void set_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
    }

    void append_flag(std::string&& flag) noexcept
    {
        m_flags.push_back(std::move(flag));
    }

    bool accepts_flag(const std::string& flag) const noexcept
    {
        return std::ranges::find(m_flags, flag) != m_flags.end();
    }

    std::string& get_name() noexcept
    {
        return m_name;
    }

    std::span<std::string>& get_flags() noexcept
    {
        return m_flags;
    }

    const std::string& get_name() const noexcept
    {
        return m_name;
    }

    const std::span<const std::string>& get_flags() const noexcept
    {
        return m_flags;
    }

private:
    std::string m_name;
    std::vector<std::string> m_flags;
};

} // namespace cc_utils::cli
