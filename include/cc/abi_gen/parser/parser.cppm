module;

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

export module cc_abi_gen_parser:parser;

import std;
import cc_utils_cli;
import :visitor;

export namespace cc_abi_gen::parser {

// Parses every C header under a directory with real Clang AST (not regex) into the registry.
class Parser
{
public:
    Parser(std::string_view compiler_path, Registry& registry) :
        m_visitor{registry, ""},
        m_tool_name{resolve_tool_name(compiler_path)}
    {
        auto result = cache_system_arguments();
        if (!result) {
            throw std::runtime_error(result.error());
        }
    }

    ~Parser() = default;
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;

    // Every `.h`/`.hpp` below `root / headers_path`; header paths are kept relative to `root`.
    std::expected<void, std::string>
    parse_directory(const std::filesystem::path& root, std::string_view headers_path)
    {
        static constexpr std::array<std::string_view, 2> EXTENSIONS = {".h", ".hpp"};

        std::vector<std::string> arguments{
            "-xc++",
            std::string{LANGUAGE_STANDARD},
            "-I" + root.string(),
        };
        arguments.insert(arguments.end(), m_system_arguments.begin(), m_system_arguments.end());

        for (const auto& entry:
             std::filesystem::recursive_directory_iterator{root / headers_path}) {
            if (!entry.is_regular_file() ||
                !std::ranges::contains(EXTENSIONS, entry.path().extension().string())) {
                continue;
            }

            std::ifstream file{entry.path()};
            if (!file) {
                return std::unexpected{"cannot open header: " + entry.path().string()};
            }
            const std::string SOURCE{std::istreambuf_iterator<char>{file}, {}};

            auto unit = clang::tooling::buildASTFromCodeWithArgs(
                SOURCE,
                arguments,
                entry.path().string(),
                m_tool_name
            );
            if (!unit) {
                return std::unexpected{"failed to parse header: " + entry.path().string()};
            }

            m_visitor.parse(
                unit->getASTContext().getTranslationUnitDecl(),
                unit->getSourceManager(),
                std::filesystem::relative(entry.path(), root).generic_string()
            );
        }

        return {};
    }

    void set_namespace_name(std::string&& namespace_name)
    {
        m_visitor.set_namespace_name(std::move(namespace_name));
    }

    [[nodiscard]] const Visitor& get_visitor() const noexcept
    {
        return m_visitor;
    }

private:
    static constexpr std::string_view LANGUAGE_STANDARD = "-std=c++26";

    // Asks clang++ for its system include directories and turns each into `-isystem<dir>`.
    std::expected<void, std::string> cache_system_arguments()
    {
        auto command = cc_utils::cli::Command{cc_utils::cli::Executable{"clang++"}};
        command.add_arguments({"-E", "-x", "c++", "-", "-v"}).add_input("");

        auto result = cc_utils::cli::Runner{}.execute(std::move(command));
        if (!result) {
            return std::unexpected{result.error()};
        }
        if (!result->success()) {
            return std::unexpected{std::format(
                "Compiler failed to dump includes. Exit code: {}. Error: {}",
                result->get_exit_code(),
                result->get_std_err()
            )};
        }

        std::istringstream stream{result->get_std_err() + "\n" + result->get_std_out()};
        std::string line;
        bool in_search_list = false;
        while (std::getline(stream, line) && !line.contains("End of search list.")) {
            if (line.contains("#include <...> search starts here")) {
                in_search_list = true;
                continue;
            }
            if (!in_search_list) {
                continue;
            }

            // Example: " /usr/include (framework directory)" -> "/usr/include"
            std::string_view directory{line};
            directory = directory.substr(0, directory.find("(framework directory)"));
            directory.remove_prefix(std::min(directory.find_first_not_of(" \t"), directory.size()));
            directory = directory.substr(0, directory.find_last_not_of(" \t\r") + 1);
            if (!directory.empty()) {
                m_system_arguments.push_back(std::format("-isystem{}", directory));
            }
        }

        return {};
    }

    static std::string resolve_tool_name(std::string_view compiler_path)
    {
        cc_utils::cli::Executable executable{std::string{compiler_path}};

        return executable.is_found() ? executable.get_path() : std::string{compiler_path};
    }

    Visitor m_visitor;
    std::vector<std::string> m_system_arguments;
    std::string m_tool_name;
};

} // namespace cc_abi_gen::parser
