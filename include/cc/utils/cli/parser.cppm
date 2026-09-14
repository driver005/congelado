export module cc_utils_cli:parser;

import std;
import :parser_schema;
import :ast_invocation;

export namespace cc_utils::cli {

class Parser
{
public:
    explicit Parser(parser::Schema&& schema) noexcept :
        m_schema{std::move(schema)}
    {
    }

    ~Parser() = default;

    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;

    Parser& add_invocation(ast::Invocation&& invocation) noexcept
    {
        m_invocation = std::move(invocation);
        return *this;
    }

    Parser& set_schema(parser::Schema&& schema) noexcept
    {
        m_schema = std::move(schema);
        return *this;
    }

    [[nodiscard]] std::expected<void, std::string> parse(int argc, const char* const* argv) noexcept
    {
        if (argc == 0 || argv == nullptr) {
            return std::unexpected("No arguments provided or argv is null");
        }

        std::span<const char* const> args(argv, argc);

        m_invocation.set_program_name(std::string(args.front()));
        args = args.subspan(1);

        ast::Command* active_cmd = nullptr;

        while (!args.empty()) {
            std::string_view current = args.front();
            args = args.subspan(1);

            if (current == "--") {
                while (!args.empty()) {
                    if (active_cmd) {
                        active_cmd->add_operand(std::string(args.front()));
                    } else {
                        m_invocation.add_global_operand(std::string(args.front()));
                    }
                    args = args.subspan(1);
                }
                break;
            }

            if (current.starts_with('-') && current.size() > 1) {
                std::size_t prefix_len = current.starts_with("--") ? 2 : 1;
                std::string_view flag_expr = current.substr(prefix_len);

                if (auto eq_pos = flag_expr.find('='); eq_pos != std::string_view::npos) {
                    add_flag(active_cmd, flag_expr.substr(0, eq_pos), flag_expr.substr(eq_pos + 1));
                } else {
                    if (!args.empty() && !std::string_view(args.front()).starts_with('-')) {
                        add_flag(active_cmd, flag_expr, args.front());
                        args = args.subspan(1);
                    } else {
                        add_flag(active_cmd, flag_expr, std::nullopt);
                    }
                }
            } else {
                if (m_schema.accepts_option(current)) {
                    ast::Command new_cmd{};
                    new_cmd.set_name(std::string(current));

                    if (!active_cmd) {
                        m_invocation.add_command(std::move(new_cmd));
                        active_cmd = &m_invocation.get_commands().back();
                    } else {
                        active_cmd->add_subcommand(std::move(new_cmd));

                        active_cmd = &active_cmd->get_subcommands().back();
                    }
                } else {
                    if (active_cmd) {
                        active_cmd->add_operand(std::string(current));
                    } else {
                        m_invocation.add_global_operand(std::string(current));
                    }
                }
            }
        }

        return {};
    }

    [[nodiscard]] std::expected<void, std::string> check()
    {
        return m_schema.validate(m_invocation);
    }

    void set_invocation(ast::Invocation&& invocation) noexcept
    {
        m_invocation = std::move(invocation);
    }

    void set_schema(parser::Schema&& schema) noexcept
    {
        m_schema = std::move(schema);
    }

    [[nodiscard]] const ast::Invocation& get_invocation() const noexcept
    {
        return m_invocation;
    }

    [[nodiscard]] const parser::Schema& get_schema() const noexcept
    {
        return m_schema;
    }


private:
    void add_flag(
        ast::Command* active_cmd,
        std::string_view name,
        std::optional<std::string_view> value
    )
    {
        ast::Flag new_flag{};
        new_flag.set_name(std::string(name));

        if (value) {
            new_flag.set_value(std::string(*value));
            new_flag.set_has_value(true);
        } else {
            new_flag.set_has_value(false);
        }

        if (active_cmd) {
            active_cmd->add_flag(std::move(new_flag));
        } else {
            m_invocation.add_global_flag(std::move(new_flag));
        }
    }

    ast::Invocation m_invocation{};
    parser::Schema m_schema{};
};

} // namespace cc_utils::cli
