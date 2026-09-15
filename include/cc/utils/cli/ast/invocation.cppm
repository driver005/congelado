export module cc_utils_cli_ast:invocation;

import std;
import :flag;
import :command;

export namespace cc_utils::cli::ast {

class Invocation
{
public:
    Invocation() = default;

    explicit Invocation(std::string&& program_name) noexcept :
        m_program_name{std::move(program_name)}
    {
    }

    Invocation(
        std::string&& program_name,
        std::vector<Flag>&& global_flags,
        std::vector<std::string>&& global_operands,
        std::vector<Command>&& commands
    ) noexcept :
        m_program_name{std::move(program_name)},
        m_global_flags{std::move(global_flags)},
        m_global_operands{std::move(global_operands)},
        m_commands{std::move(commands)}
    {
    }

    ~Invocation() = default;
    Invocation(const Invocation&) = delete;
    Invocation& operator=(const Invocation&) = delete;
    Invocation(Invocation&&) = default;
    Invocation& operator=(Invocation&&) = default;

    Invocation& add_program_name(std::string&& program_name) noexcept
    {
        m_program_name = std::move(program_name);
        return *this;
    }

    Invocation& add_global_flag(Flag&& flag) noexcept
    {
        m_global_flags.push_back(std::move(flag));
        return *this;
    }

    Invocation& add_global_operand(std::string&& operand)
    {
        m_global_operands.push_back(std::move(operand));
        return *this;
    }

    Invocation& add_command(Command&& cmd)
    {
        m_commands.push_back(std::move(cmd));
        return *this;
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const Command>>
    current_command() const noexcept
    {
        if (m_commands.empty()) {
            return std::nullopt;
        }
        return std::ref(m_commands.back());
    }

    void append_global_flag(Flag&& flag) noexcept
    {
        m_global_flags.push_back(std::move(flag));
    }

    void set_program_name(std::string&& program_name) noexcept
    {
        m_program_name = std::move(program_name);
    }

    void append_global_operand(std::string&& operand) noexcept
    {
        m_global_operands.emplace_back(std::move(operand));
    }

    void append_command(Command&& cmd) noexcept
    {
        m_commands.emplace_back(std::move(cmd));
    }

    [[nodiscard]] const std::string& get_program_name() const noexcept
    {
        return m_program_name;
    }

    [[nodiscard]] std::span<const Flag> get_global_flags() const noexcept
    {
        return m_global_flags;
    }

    // Ther user has to be able to change flags
    [[nodiscard]] std::span<Flag> get_global_flags() noexcept
    {
        return m_global_flags;
    }

    [[nodiscard]] std::span<const Command> get_commands() const noexcept
    {
        return m_commands;
    }

    // Ther user has to be able to change flags in command
    [[nodiscard]] std::span<Command> get_commands() noexcept
    {
        return m_commands;
    }

    [[nodiscard]] std::span<const std::string> get_global_operands() const noexcept
    {
        return m_global_operands;
    }

private:
    std::string m_program_name{};
    std::vector<Flag> m_global_flags{};
    std::vector<std::string> m_global_operands{};
    std::vector<Command> m_commands{};
};

} // namespace cc_utils::cli::ast
