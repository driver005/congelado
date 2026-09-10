export module cc_utils_cli:ast_command;

import std;
import :ast_flag;

export namespace cc_utils::cli::ast {

class Command
{
public:
    Command() = default;

    explicit Command(std::string&& name) noexcept :
        m_name{std::move(name)}
    {
    }

    Command(
        std::string&& name,
        std::vector<Flag>&& flags,
        std::vector<Command>&& subcommands,
        std::vector<std::string>&& operands
    ) noexcept :
        m_name{std::move(name)},
        m_flags{std::move(flags)},
        m_subcommands{std::move(subcommands)},
        m_operands{std::move(operands)}
    {
    }

    Command& add_flag(Flag&& flag)
    {
        m_flags.push_back(std::move(flag));
        return *this;
    }

    Command& add_subcommand(Command&& cmd)
    {
        m_subcommands.push_back(std::move(cmd));
        return *this;
    }

    Command& add_operand(std::string&& operand)
    {
        m_operands.push_back(std::move(operand));
        return *this;
    }

    void set_name(std::string&& name) noexcept
    {
        m_name = std::move(name);
    }

    [[nodiscard]] std::string& get_name() noexcept
    {
        return m_name;
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] std::span<Flag> get_flags() noexcept
    {
        return m_flags;
    }

    [[nodiscard]] std::span<const Flag> get_flags() const noexcept
    {
        return m_flags;
    }

    [[nodiscard]] std::span<Command> get_subcommands() noexcept
    {
        return m_subcommands;
    }

    [[nodiscard]] std::span<const Command> get_subcommands() const noexcept
    {
        return m_subcommands;
    }

    [[nodiscard]] std::span<std::string> get_operands() noexcept
    {
        return m_operands;
    }

    [[nodiscard]] std::span<const std::string> get_operands() const noexcept
    {
        return m_operands;
    }

private:
    std::string m_name{};
    std::vector<Flag> m_flags{};
    std::vector<Command> m_subcommands{};
    std::vector<std::string> m_operands{};
};

} // namespace cc_utils::cli::ast
