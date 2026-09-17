module;

#include <clang/AST/DeclCXX.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <span>
#include <string>
#include <string_view>

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

    ~Parser() = default;
    Parser(const Parser&) = delete;
    Parser& operator=(const Parser&) = delete;
    Parser(Parser&&) = default;
    Parser& operator=(Parser&&) = default;

    Parser& add_registry(Registry&& registry) noexcept
    {
        m_registry = std::move(registry);
        return *this;
    }

    Parser& add_include_finder(helper::IncludeFinder&& finder) noexcept
    {
        m_include_finder = std::move(finder);
        return *this;
    }

    Parser& add_visitor(vtable::AstVisitor&& visitor) noexcept
    {
        m_visitor = std::move(visitor);
        return *this;
    }

    Parser& add_system_argument(std::string&& argument) noexcept
    {
        m_system_arguments.push_back(std::move(argument));
        return *this;
    }

    Parser& add_domain(std::string&& domain) noexcept
    {
        m_domain = std::move(domain);
        return *this;
    }

    Parser& add_tool_name(std::string&& tool_name) noexcept
    {
        m_tool_name = std::move(tool_name);
        return *this;
    }

    std::expected<void, std::string> parse_directory(
        const std::filesystem::path& root_path,
        const std::string_view headers_path,
        std::span<const std::string_view> extensions
    )
    {
        for (const auto& entry:
             std::filesystem::recursive_directory_iterator{root_path / headers_path}) {
            // Skip directories, symlinks, etc.
            if (!entry.is_regular_file()) {
                continue;
            }

            // Skip non-header files (like README.md or BUILD)
            auto ext = entry.path().extension();
            if (!std::ranges::contains(extensions, ext)) {
                std::println("Skipping {}", entry.path().string());
                continue;
            }

            auto model = parse_file(entry.path(), root_path);

            if (!model) {
                return std::unexpected{std::move(model.error())};
            }
        }

        return {};
    }

    std::expected<void, std::string>
    parse_file(const std::filesystem::path& file_path, const std::filesystem::path& root_path)
    {
        auto source = open_file(file_path);
        if (!source.has_value()) {
            return std::unexpected{std::move(source.error())};
        }

        auto arguments = parse_arguments(root_path);
        if (!arguments.has_value()) {
            return std::unexpected{std::move(arguments.error())};
        }

        auto translation_unit = clang::tooling::buildASTFromCodeWithArgs(
            *source,
            *arguments,
            file_path.string(),
            m_tool_name
        );
        if (!translation_unit) {
            return std::unexpected{"failed to parse header: " + file_path.string()};
        }

        auto* tu_decl = translation_unit->getASTContext().getTranslationUnitDecl();
        auto& sm = translation_unit->getASTContext().getSourceManager();

        auto header_path = remove_base_path(file_path, root_path);
        if (!header_path.has_value()) {
            return std::unexpected{std::move(header_path.error())};
        }

        auto nothing = collect_records(tu_decl, *header_path, sm);
        if (nothing) {
            // TODO: make warning only
            // return std::unexpected{"no vtable struct found in: " + header_path.string()};
            std::println("no vtable struct found in: {}", file_path.string());
        }

        return {};
    }

    void set_registry(Registry&& registry) noexcept
    {
        m_registry = std::move(registry);
    }

    void set_include_finder(helper::IncludeFinder&& finder) noexcept
    {
        m_include_finder = std::move(finder);
    }

    void set_visitor(vtable::AstVisitor&& visitor) noexcept
    {
        m_visitor = std::move(visitor);
    }

    void append_system_argument(std::string&& argument) noexcept
    {
        m_system_arguments.push_back(std::move(argument));
    }

    void set_domain(std::string&& domain) noexcept
    {
        m_domain = std::move(domain);
    }

    void set_tool_name(std::string&& tool_name) noexcept
    {
        m_tool_name = std::move(tool_name);
    }

    const Registry& get_registry() const noexcept
    {
        return m_registry;
    }

    const helper::IncludeFinder& get_include_finder() const noexcept
    {
        return m_include_finder;
    }

    const vtable::AstVisitor& get_ast_visitor() const noexcept
    {
        return m_visitor;
    }

    std::span<const std::string> get_system_argument() const noexcept
    {
        return m_system_arguments;
    }

    const std::string& get_domain() const noexcept
    {
        return m_domain;
    }

    const std::string& get_tool_name() const noexcept
    {
        return m_tool_name;
    }

private:
    bool collect_records(
        clang::DeclContext* context,
        const std::string_view header_path,
        const clang::SourceManager& sm
    )
    {
        bool nothing = true;

        for (clang::Decl* decl: context->decls()) {
            // Skip declarations from included files — only register structs defined in the current
            // file
            if (!sm.isInMainFile(decl->getLocation())) {
                continue;
            }

            if (auto* record_decl = clang::dyn_cast<clang::RecordDecl>(decl)) {
                auto model = m_visitor.traverse_record_decl(record_decl, header_path);
                if (model.has_value()) {
                    nothing = false;
                    m_registry.append_model(std::move(*model));
                }
            } else if (auto* spec_decl = clang::dyn_cast<clang::LinkageSpecDecl>(decl)) {
                if (!collect_records(spec_decl, header_path, sm)) {
                    nothing = false;
                }
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
        auto cmd = cc_utils::cli::Command{cc_utils::cli::Executable{"clang++"}};
        cmd.add_arguments({"-E", "-x", "c++", "-", "-v"}).add_input("");

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

    std::expected<std::string, std::string> remove_base_path(
        const std::filesystem::path& header_path,
        const std::filesystem::path& base_path
    ) const
    {
        // Normalize to strip trailing slashes or redundant '.' segments
        auto norm_header = header_path.lexically_normal();
        auto norm_base = base_path.lexically_normal();

        // Range-first: Search for base_path components inside header_path
        auto match = std::ranges::search(norm_header, norm_base);
        if (match.empty()) {
            return std::unexpected(std::string{"header path does not contain base path"});
        }

        // Accumulate the remaining components after the matched base path
        std::filesystem::path rest;
        for (const auto& comp: std::ranges::subrange(match.end(), norm_header.end())) {
            rest /= comp;
        }

        return rest.generic_string();
    }

    Registry m_registry;
    helper::IncludeFinder m_include_finder;
    vtable::AstVisitor m_visitor;
    std::vector<std::string> m_system_arguments;
    std::string m_domain;
    std::string m_tool_name;
};

} // namespace cc_abi_gen::parser
