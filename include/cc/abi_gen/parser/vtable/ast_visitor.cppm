module;

#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>

export module cc_abi_gen_parser:vtable_ast_visitor;

import std;
import :slot_slot;
import :vtable_model;
import :vtable_naming;
import :slot_reader;

export namespace cc_abi_gen::parser::vtable {

class AstVisitor : public clang::RecursiveASTVisitor<AstVisitor>
{
public:
    AstVisitor() = default;

    ~AstVisitor() = default;
    AstVisitor(const AstVisitor&) = delete;
    AstVisitor& operator=(const AstVisitor&) = delete;
    AstVisitor(AstVisitor&&) = default;
    AstVisitor& operator=(AstVisitor&&) = default;

    AstVisitor& add_nameing(Naming&& naming) noexcept
    {
        m_naming = std::move(naming);
        return *this;
    }

    AstVisitor& add_slot_reader(slot::Reader&& slot_reader) noexcept
    {
        m_reader = std::move(slot_reader);
        return *this;
    }

    std::expected<Model, std::string>
    traverse_record_decl(clang::RecordDecl* record_decl, const std::string_view header_path)
    {
        // Check that the struct is a declation as well
        if (!record_decl->isCompleteDefinition()) {
            return std::unexpected("Record declaration is not a complete definition");
        }

        // Returns an iterator pointing to the member variables of the record.
        auto field_iterator = record_decl->field_begin();
        if (field_iterator == record_decl->field_end()) {
            return std::unexpected("Record declaration has no fields");
        }

        clang::FieldDecl* first_field = *field_iterator;

        // Extracts the identifier name of the declaration
        if (first_field->getNameAsString() != "struct_size") {
            return std::unexpected("Record declaration does not have a struct_size field");
        }

        std::vector<slot::Slot> slots;

        for (++field_iterator; field_iterator != record_decl->field_end(); ++field_iterator) {
            clang::FieldDecl* field = *field_iterator;

            // Retrieves the Clang object representing the actual C++ data type of the field.
            if (!field->getType()->isFunctionPointerType()) {
                continue;
            }

            slots.push_back(m_reader.read(field));
        }

        if (slots.empty()) {
            return std::unexpected("Record declaration does not have any slots");
        }

        std::string struct_name = record_decl->getNameAsString();
        auto domain = extract_domain(header_path);
        if (!domain.has_value()) {
            return std::unexpected("Could not extract domain name from header path");
        }

        auto domain_name = std::string{domain.value()};


        std::println("[cc_abi_gen] parsing: {} (domain {})", header_path, domain_name);

        return Model{
            std::move(struct_name),
            std::move(m_naming.struct_size_macro(struct_name)),
            std::move(domain_name),
            std::move(m_naming.class_name(struct_name)),
            std::string{header_path},
            std::move(slots)
        };
    }

    void set_naming(Naming&& naming) noexcept
    {
        m_naming = std::move(naming);
    }

    void set_slot_reader(slot::Reader&& slot_reader) noexcept
    {
        m_reader = std::move(slot_reader);
    }

    const Naming& get_naming() const noexcept
    {
        return m_naming;
    }

    const slot::Reader& get_slot_reader() const noexcept
    {
        return m_reader;
    }


private:
    std::expected<std::string, std::string> extract_domain(std::string_view header_path) const
    {
        auto rev_path = header_path | std::views::reverse;
        auto last_slash = std::ranges::find(rev_path, '/');

        if (last_slash == rev_path.end()) {
            return std::string{};
        }

        auto second_slash = std::ranges::find(std::ranges::next(last_slash), rev_path.end(), '/');

        std::string_view domain_view;
        if (second_slash == rev_path.end()) {
            // Only 1 slash: from the char right after the '/' to the end
            domain_view = std::string_view(last_slash.base(), header_path.end());
        } else {
            // 2+ slashes: from the char right after the 2nd-to-last '/', up to the last '/'
            domain_view = std::string_view(second_slash.base(), last_slash.base() - 1);
        }

        // Strip extension natively using Range-first iterators
        auto rev_domain = domain_view | std::views::reverse;
        auto dot = std::ranges::find(rev_domain, '.');

        if (dot != rev_domain.end()) {
            // dot.base() - 1 points exactly to the '.'
            domain_view = std::string_view(domain_view.begin(), dot.base() - 1);
        }

        return std::string{domain_view};
    }

    Naming m_naming;
    slot::Reader m_reader;
};

} // namespace cc_abi_gen::parser::vtable
