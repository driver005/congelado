module;

#include <clang/AST/DeclCXX.h>
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
        m_domain{domain},
        m_tool_name{resolve_tool_name(compiler_path)}
    {
        auto result = cache_system_arguments();
        if (!result) {
            throw std::runtime_error(result.error());
        }
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
        if (!source.has_value()) {
            return std::unexpected{std::move(source.error())};
        }

        auto arguments = parse_arguments(include_root);
        if (!arguments.has_value()) {
            return std::unexpected{std::move(arguments.error())};
        }

        auto translation_unit = clang::tooling::buildASTFromCodeWithArgs(
            *source,
            *arguments,
            header_path.string(),
            m_tool_name
        );
        if (!translation_unit) {
            return std::unexpected{"failed to parse header: " + header_path.string()};
        }

        auto* tu_decl = translation_unit->getASTContext().getTranslationUnitDecl();

        auto nothing = collect_records(tu_decl);
        if (nothing) {
            return std::unexpected{"no vtable struct found in: " + header_path.string()};
        }

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
    bool collect_records(clang::DeclContext* context)
    {
        bool nothing = true;

        // Iterate through all top-level declarations in the file
        for (clang::Decl* decl: tu_decl->decls()) {
            // Check if the declaration is a struct/class (RecordDecl)
            if (auto* record_decl = clang::dyn_cast<clang::RecordDecl>(decl)) {
                auto model = m_visitor.traverse_record_decl(record_decl);
                if (model.has_value()) {
                    nothing = false;
                    m_registry.add(std::move(*model));
                }
                // Check if it's an extern "C" block containing nested declarations
            } else if (auto* spec_decl = clang::dyn_cast<clang::LinkageSpecDecl>(decl)) {
                return collect_records(spec_decl);
            }
        }

        return nothing;
    }

    std::expected<std::string, std::string> open_file(const std::filesystem::path& header_path)
    {
        std::ifstream file{header_path};
        if (!file) {
            return std::unexpected{"cannot open header: " + header_path.string()};
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
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

        arguments.insert(arguments.end(), m_system_arguments.begin(), m_system_arguments.end());

        return arguments;
    }

    std::expected<void, std::string> cache_system_arguments()
    {
        auto cmd = cc_utils::cli::Command{cc_utils::cli::Executable{"clang++"}}
                       .arguments({"-E", "-x", "c++", "-", "-v"})
                       .input("");

        std::vector<std::string> directories;

        auto result = m_include_finder.discover_from_command(std::move(cmd), directories);
        if (!result) {
            return std::unexpected{result.error()};
        }

        for (const std::string& directory: directories) {
            m_system_arguments.push_back(std::format("-isystem{}", directory));
        }

        return {};
    }

    static std::string resolve_tool_name(std::string_view compiler_path)
    {
        cc_utils::cli::Executable executable{std::string{compiler_path}};
        return executable.is_found() ? executable.get_path() : std::string{compiler_path};
    }

    Registry m_registry;
    helper::IncludeFinder m_include_finder;
    vtable::AstVisitor m_visitor;
    std::vector<std::string> m_system_arguments;
    std::string m_domain;
    std::string m_tool_name;
};

} // namespace cc_abi_gen::parser
