module;

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>

#include <cctype>

export module cc_abi_gen_parser:value_visitor;

import std;
import :value_field;
import :value_model;
import :vtable_naming;

export namespace cc_abi_gen::parser::value {

// Reads plain C structs and enums, the ones that carry no vtable slots and no plugin handle.
class Visitor
{
public:
    Visitor() = default;
    ~Visitor() = default;
    Visitor(const Visitor&) = delete;
    Visitor& operator=(const Visitor&) = delete;
    Visitor(Visitor&&) = default;
    Visitor& operator=(Visitor&&) = default;

    std::expected<Model, std::string>
    traverse_record_decl(clang::RecordDecl* record_decl, const std::string_view header_path) const
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
            fields.push_back(read_field(field));
        }

        Model model{
            Kind::Struct,
            record_decl->getNameAsString(),
            vtable::value_class_name(record_decl->getNameAsString()),
            domain_of(header_path),
            std::string{header_path}
        };
        for (Field& field: fields) {
            model.add_field(std::move(field));
        }
        model.add_struct_size(struct_size);

        return model;
    }

    std::expected<Model, std::string>
    traverse_enum_decl(clang::EnumDecl* enum_decl, const std::string_view header_path) const
    {
        if (!enum_decl->isCompleteDefinition() || enum_decl->getNameAsString().empty()) {
            return std::unexpected("Enum is not a named complete definition");
        }

        Model model{
            Kind::Enum,
            enum_decl->getNameAsString(),
            vtable::value_class_name(enum_decl->getNameAsString()),
            domain_of(header_path),
            std::string{header_path}
        };
        for (clang::EnumConstantDecl* constant: enum_decl->enumerators()) {
            model.add_enumerator(constant->getNameAsString());
        }

        return model;
    }

private:
    // Members of unnamed struct or union type have no spellable C++ name; get_handle() reaches them.
    static bool has_anonymous_type(clang::QualType type)
    {
        const std::string spelling = type.getAsString();

        return spelling.contains("unnamed") || spelling.contains("anonymous");
    }

    Field read_field(clang::FieldDecl* field) const
    {
        clang::QualType type = field->getType();
        std::size_t extent = 0;
        if (const auto* array = field->getASTContext().getAsConstantArrayType(type)) {
            extent = static_cast<std::size_t>(array->getSize().getZExtValue());
            type = array->getElementType();
        }

        const bool record = type->isRecordType();

        return Field{field->getNameAsString(), spell(type.getAsString()), extent, record};
    }

    // Writes global names with `::` and fixed width integers with `std::`.
    static std::string spell(const std::string& type)
    {
        static constexpr std::array<std::string_view, 12> k_std_names{
            "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t",
            "uint32_t", "uint64_t", "intptr_t", "uintptr_t", "size_t", "ptrdiff_t"
        };
        static constexpr std::array<std::string_view, 5> k_keywords{
            "const", "struct", "enum", "union", "volatile"
        };

        std::string result;
        std::size_t index = 0;
        while (index < type.size()) {
            if (!std::isalpha(static_cast<unsigned char>(type[index])) && type[index] != '_') {
                result += type[index++];
                continue;
            }

            std::size_t end = index;
            while (end < type.size() &&
                   (std::isalnum(static_cast<unsigned char>(type[end])) || type[end] == '_')) {
                ++end;
            }
            const std::string_view token{type.data() + index, end - index};
            if (std::ranges::find(k_std_names, token) != k_std_names.end()) {
                result += "std::";
            } else if (std::ranges::find(k_keywords, token) == k_keywords.end() &&
                       token.starts_with("TF")) {
                result += "::";
            }
            result += token;
            index = end;
        }

        return result;
    }

    // The folder that holds the header. Example: include/c/extern/stream_executor/types.h
    static std::string domain_of(std::string_view header_path)
    {
        return std::filesystem::path{header_path}.parent_path().filename().generic_string();
    }
};

} // namespace cc_abi_gen::parser::value
