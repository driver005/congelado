module;

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>

export module cc_abi_gen_parser:visitor;

import std;
import :model;
import :slot_slot;
import :slot_reader;
import :field;
import :utils;

export namespace cc_abi_gen::parser {

class Visitor : public clang::RecursiveASTVisitor<Visitor>
{
public:
    Visitor() = default;

    ~Visitor() = default;
    Visitor(const Visitor&) = delete;
    Visitor& operator=(const Visitor&) = delete;
    Visitor(Visitor&&) = default;
    Visitor& operator=(Visitor&&) = default;

    Visitor& add_slot_reader(slot::Reader&& slot_reader) noexcept
    {
        m_reader = std::move(slot_reader);
        return *this;
    }

    void set_slot_reader(slot::Reader&& slot_reader) noexcept
    {
        m_reader = std::move(slot_reader);
    }

    [[nodiscard]] const slot::Reader& get_slot_reader() const noexcept
    {
        return m_reader;
    }

    // Single entry: record → vtable shape first, plain struct fallback; enum → enum model.
    std::expected<Model, std::string>
    traverse(clang::Decl* decl, const std::string_view header_path)
    {
        if (auto* record_decl = llvm::dyn_cast<clang::RecordDecl>(decl)) {
            if (auto vtable = traverse_vtable(record_decl, header_path)) {
                return vtable;
            }
            return traverse_value(record_decl, header_path);
        }

        if (auto* enum_decl = llvm::dyn_cast<clang::EnumDecl>(decl)) {
            return traverse_enum(enum_decl, header_path);
        }

        return std::unexpected("Decl is neither a record nor an enum");
    }

private:
    // Ops vtable struct: struct_size first + function pointer slots.
    std::expected<Model, std::string>
    traverse_vtable(clang::RecordDecl* record_decl, const std::string_view header_path)
    {
        if (!record_decl->isCompleteDefinition()) {
            return std::unexpected("Record declaration is not a complete definition");
        }

        auto field_iterator = record_decl->field_begin();
        if (field_iterator == record_decl->field_end()) {
            return std::unexpected("Record declaration has no fields");
        }

        clang::FieldDecl* first_field = *field_iterator;

        if (first_field->getNameAsString() != "struct_size") {
            return std::unexpected("Record declaration does not have a struct_size field");
        }

        std::vector<slot::Slot> slots;

        for (++field_iterator; field_iterator != record_decl->field_end(); ++field_iterator) {
            clang::FieldDecl* field = *field_iterator;

            if (!field->getType()->isFunctionPointerType()) {
                continue;
            }

            auto slot = m_reader.read(field);
            if (!slot.get_name().empty()) {
                slots.push_back(std::move(slot));
            }
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
            struct_size_macro(struct_name),
            std::move(domain_name),
            class_name(struct_name),
            std::string{header_path},
            std::move(slots)
        };
    }

    // Plain C struct: no vtable slots, no plugin handle.
    std::expected<Model, std::string>
    traverse_value(clang::RecordDecl* record_decl, const std::string_view header_path) const
    {
        if (!record_decl->isStruct() || !record_decl->isCompleteDefinition()) {
            return std::unexpected("Record is not a complete struct");
        }
        if (record_decl->getNameAsString().empty() || record_decl->field_empty()) {
            return std::unexpected("Record has no name or no fields");
        }

        bool struct_size = false;
        std::vector<Field> fields;
        for (clang::FieldDecl* field: record_decl->fields()) {
            const std::string name = field->getNameAsString();
            if (name == "plugin_data" || field->getType()->isFunctionPointerType()) {
                return std::unexpected("Record is a handle or a vtable");
            }
            if (name == "struct_size") {
                struct_size = true;
                continue;
            }
            if (name == "ext" || name.empty() || has_anonymous_type(field->getType())) {
                continue;
            }

            Field parsed;
            parsed.read_clang(field);
            fields.push_back(std::move(parsed));
        }

        Model model{
            Kind::STRUCT,
            record_decl->getNameAsString(),
            value_class_name(record_decl->getNameAsString()),
            to_file_path(header_path),
            std::string{header_path}
        };
        for (Field& field: fields) {
            model.add_field(std::move(field));
        }
        model.add_struct_size(struct_size);

        return model;
    }

    std::expected<Model, std::string>
    traverse_enum(clang::EnumDecl* enum_decl, const std::string_view header_path) const
    {
        if (!enum_decl->isCompleteDefinition() || enum_decl->getNameAsString().empty()) {
            return std::unexpected("Enum is not a named complete definition");
        }

        Model model{
            Kind::ENUM,
            enum_decl->getNameAsString(),
            value_class_name(enum_decl->getNameAsString()),
            to_file_path(header_path),
            std::string{header_path}
        };
        for (clang::EnumConstantDecl* constant: enum_decl->enumerators()) {
            model.add_enumerator(constant->getNameAsString());
        }

        return model;
    }

    static std::expected<std::string, std::string> extract_domain(std::string_view header_path)
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

    // Members of unnamed struct or union type have no spellable C++ name; get_handle() reaches
    // them.
    static constexpr bool has_anonymous_type(clang::QualType type)
    {
        const std::string SPELLING = type.getAsString();

        return SPELLING.contains("unnamed") || SPELLING.contains("anonymous");
    }

    // Example: include/c/intern/string.h
    static constexpr std::string to_file_path(std::string_view header_path)
    {
        return std::filesystem::path{header_path}.parent_path().filename().generic_string();
    }

    slot::Reader m_reader{};
};

} // namespace cc_abi_gen::parser
