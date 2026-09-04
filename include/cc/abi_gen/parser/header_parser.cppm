module;

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>

export module cc_abi_gen_parser:header_parser;

import std;
import :vtable_model;
import :vtable_ast_visitor;
import :system_include_finder;

export namespace cc_abi_gen {

// Parses one include/c/extern/<domain>/<domain>.h vtable header with real Clang AST (not regex).
class HeaderParser
{
public:
    std::expected<VtableModel, std::string> parse(
        const std::filesystem::path &header_path, const std::filesystem::path &include_root)
    {
        std::ifstream file(header_path);
        if (!file) {

            return std::unexpected{"cannot open header: " + header_path.string()};
        }

        m_scratch_source.clear();
        std::stringstream buffer;
        buffer << file.rdbuf();
        m_scratch_source = buffer.str();

        m_scratch_arguments.clear();
        m_scratch_arguments.push_back("-xc++");
        m_scratch_arguments.push_back("-std=c++23");
        m_scratch_arguments.push_back("-I" + include_root.string());
        for (const std::string &directory : m_include_finder.discover()) {

            m_scratch_arguments.push_back("-isystem");
            m_scratch_arguments.push_back(directory);
        }

        auto translation_unit = clang::tooling::buildASTFromCodeWithArgs(
            m_scratch_source, m_scratch_arguments, header_path.string());
        if (!translation_unit) {

            return std::unexpected{"failed to parse header: " + header_path.string()};
        }

        m_visitor.m_model.reset();
        m_visitor.TraverseDecl(translation_unit->getASTContext().getTranslationUnitDecl());

        if (!m_visitor.m_model) {

            return std::unexpected{"no vtable struct found in: " + header_path.string()};
        }

        return std::move(*m_visitor.m_model);
    }

private:
    SystemIncludeFinder m_include_finder;
    VtableAstVisitor m_visitor;
    std::vector<std::string> m_scratch_arguments;
    std::string m_scratch_source;
};

} // namespace cc_abi_gen
