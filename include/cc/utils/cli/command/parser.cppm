module;
#include <unordered_map>
export module cc_utils_cli:parser;

import std;
import :arguments;
import :command_schema;

export namespace cc_utils::cli {

// Parses a process's own argv (argv[0] program name, argv[1] command, `--flag [value]` pairs
// after that) against a registered set of commands and their allowed flags — the receiving
// counterpart to Arguments/Command, which only ever build an argv to hand to a spawned child
// process.
class Parser
{
public:
    explicit Parser(std::vector<CommandSchema>&& commands) :
        m_commands{std::move(commands)}
    {
    }

    ~Parser() = default;
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;

    Parser& add_command_schema(CommandSchema&& command) noexcept
    {
        m_commands.push_back(std::move(command));
        return *this;
    }

    Parser& add_program_name(std::string&& program_name) noexcept
    {
        m_program_name = std::move(program_name);
        return *this;
    }

    Parser& add_command(std::string&& command) noexcept
    {
        m_command = std::move(command);
        return *this;
    }

    Parser& add_flag(std::string&& flag, std::string&& description) noexcept
    {
        m_flags.append({std::move(flag), std::move(description)});
        return *this;
    }

    std::expected<void, std::string> parse(const Arguments& arguments) noexcept
    {
        const std::vector<std::string>& args = arguments.get_args();

        m_command.clear();
        m_flags.clear();

        if (!args.empty()) {
            m_program_name = args.front();
        }

        if (args.size() < 2) {
            return std::unexpected{"missing command"};
        }

        m_command = args[1];

        const CommandSchema* schema = find_command(m_command);
        if (schema == nullptr) {
            return std::unexpected{std::format("unknown command: {}", m_command)};
        }

        for (std::size_t index = 2; index < args.size(); ++index) {
            const std::string& argument = args[index];

            if (!argument.starts_with("--")) {
                continue;
            }

            std::string name = argument.substr(2);

            if (!schema->accepts_flag(name)) {
                return std::unexpected{
                    std::format("unknown flag --{} for command {}", name, m_command)
                };
            }

            if (index + 1 < args.size() && !args[index + 1].starts_with("--")) {
                m_flags.insert_or_assign(std::move(name), args[index + 1]);
                ++index;
            } else {
                m_flags.insert_or_assign(std::move(name), std::string{});
            }
        }

        return {};
    }

    bool has_flag(const std::string& name) const noexcept
    {
        return m_flags.contains(name);
    }

    std::optional<std::string> has_value(const std::string& name) const noexcept
    {
        auto iterator = m_flags.find(name);
        if (iterator == m_flags.end()) {
            return std::nullopt;
        }

        return iterator->second;
    }

    void append_command(CommandSchema&& command) noexcept
    {
        m_commands.emplace_back(std::move(command));
    }

    void set_program_name(std::string&& program_name) noexcept
    {
        m_program_name = std::move(program_name);
    }

    void set_command(std::string&& command) noexcept
    {
        m_command = std::move(command);
    }

    void append_flag(std::string&& flag, std::string&& description) noexcept
    {
        m_flags.emplace(std::move(flag), std::move(description));
    }

    const std::string& get_program_name() const noexcept
    {
        return m_program_name;
    }

    const std::string& get_command() const noexcept
    {
        return m_command;
    }

    const std::span<const CommandSchema>& get_commands() const noexcept
    {
        return m_commands;
    }

    const std::unordered_map<std::string, std::string>& get_flags() const noexcept
    {
        return m_flags;
    }

private:
    const CommandSchema* find_command(const std::string& name) const
    {
        for (const CommandSchema& command: m_commands) {
            if (command.get_name() == name) {
                return &command;
            }
        }

        return nullptr;
    }

    std::vector<CommandSchema> m_commands;
    std::string m_program_name;
    std::string m_command;
    std::unordered_map<std::string, std::string> m_flags;
};

} // namespace cc_utils::cli
