module;

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

export module cc_abi_gen_parser:parser;

import std;
import :registry;
import :vtable_model;
import :vtable_ast_visitor;
import :helper_include_finder;

export namespace cc_abi_gen::parser {

// Parses one include/c/extern/<domain>/<domain>.h vtable header with real Clang AST (not regex).
class Parser
{
public:
    Parser(std::string_view compiler_path, std::string_view domain) :
        m_include_finder{},
        m_domain{domain},
    {
        cache_system_arguments();
    };

    std::expected<void, std::string> parse_directory(const std::filesystem::path& directory_path)
    {
        for (const auto& header_path: std::filesystem::directory_iterator{directory_path}) {
            auto model = parse_file(header_path.path(), directory_path);

            if (!model.has_value()) {
                return std::unexpected{std::move(model.error())};
            }
        }

        return {};
    }

    std::expected<void, std::string>
    parse_file(const std::filesystem::path& header_path, const std::filesystem::path& include_root)
    {
        auto source = open_file(header_path);

        auto arguments = parse_arguments(include_root);

        // Build the AST from the source code
        auto translation_unit =
            clang::tooling::buildASTFromCodeWithArgs(source, arguments, header_path.string());
        if (!translation_unit) {
            return std::unexpected{"failed to parse header: " + header_path.string()};
        }

        // Retrieves the absolute root node of Clang's Abstract Syntax Tree
        auto model = m_visitor.traverse_record_decl(
            translation_unit->getASTContext().getTranslationUnitDecl()
        );

        if (!model.has_value()) {
            return std::unexpected{"no vtable struct found in: " + header_path.string()};
        }

        m_registry.add(std::move(*model));

        return {};
    }

    Registry& get_registry()
    {
        return m_registry;
    }

    const Registry& get_registry() const
    {
        return m_registry;
    }

private:
    std::expected<std::string, std::string> open_file(const std::filesystem::path& header_path)
    {
        std::ifstream file{header_path};
        if (!file) {
            return std::unexpected{"cannot open header: " + header_path.string()};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto source = buffer.str();

        return source;
    }

    std::expected<std::vector<std::string>, std::string>
    parse_arguments(const std::filesystem::path& include_root)
    {
        // TODO: make this a const so we can update the c++ version easily
        std::vector<std::string> arguments{
            "-xc++",
            "-std=c++26",
            std::format("-I{}", include_root.string())
        };

        arguments.insert(arguments.end(), m_cached_system_args.begin(), m_cached_system_args.end());

        return arguments;
    }

    void cache_system_arguments()
    {
        auto cmd = utils::Command(utils::Executable{"clang++"})
                       .arguments({"-E", "-x", "c++", "-", "-v"})
                       .input("");

        std::vector<std::string> directories;

        auto result = m_include_finder.discover_from_command(cmd, directories);
        if (!result) {
            return std::unexpected{result.error()};
        }

        for (const std::string& directory: directories) {
            m_system_arguments.push_back(std::format("-isystem{}", directory));
        }
    }

    Registry m_registry;
    helper::IncludeFinder m_include_finder;
    vtable::AstVisitor m_visitor;
    std::vector<std::string> m_system_arguments;
    std::string m_domain;
};

} // namespace cc_abi_gen::parser
