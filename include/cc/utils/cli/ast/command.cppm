export module cc_utils_cli_ast:command;

import std;
import :flag;

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

    ~Command() = default;
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;
    Command(Command&&) = default;
    Command& operator=(Command&&) = default;

    Command& add_name(std::string&& name)
    {
        m_name = std::move(name);
        return *this;
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

    void append_flag(Flag&& flag) noexcept
    {
        m_flags.push_back(std::move(flag));
    }

    void append_subcommand(Command&& subcommand) noexcept
    {
        m_subcommands.push_back(std::move(subcommand));
    }

    void append_operand(std::string&& operand) noexcept
    {
        m_operands.push_back(std::move(operand));
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    [[nodiscard]] std::span<const Flag> get_flags() const noexcept
    {
        return m_flags;
    }

    // The user has to be able to verify the flags
    [[nodiscard]] std::span<Flag> get_flags() noexcept
    {
        return m_flags;
    }

    [[nodiscard]] std::span<const Command> get_subcommands() const noexcept
    {
        return m_subcommands;
    }

    // The user has to be able to verify the subcommands
    [[nodiscard]] std::span<Command> get_subcommands() noexcept
    {
        return m_subcommands;
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
