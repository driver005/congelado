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
    AstVisitor(Naming naming, slot::Reader slot_reader) :
        m_naming(std::move(naming)),
        m_reader(std::move(slot_reader))
    {
    }

    std::optional<Model> traverse_record_decl(clang::RecordDecl* record_decl)
    {
        // Check that the struct is a declation as well
        if (!record_decl->isCompleteDefinition()) {
            return std::nullopt;
        }

        // Returns an iterator pointing to the member variables of the record.
        auto field_iterator = record_decl->field_begin();
        if (field_iterator == record_decl->field_end()) {
            return std::nullopt;
        }

        clang::FieldDecl* first_field = *field_iterator;

        // Extracts the identifier name of the declaration
        if (first_field->getNameAsString() != "struct_size") {
            return std::nullopt;
        }

        std::vector<Slot> slots;

        for (++field_iterator; field_iterator != record_decl->field_end(); ++field_iterator) {
            clang::FieldDecl* field = *field_iterator;

            // Retrieves the Clang object representing the actual C++ data type of the field.
            if (!field->getType()->isFunctionPointerType()) {
                continue;
            }

            slots.push_back(m_reader.read(field));
        }

        return Model{
            record_decl->getNameAsString(),
            m_naming.struct_size_macro(model.m_struct_name),
            m_naming.domain_name(model.m_struct_name),
            m_naming.class_name(model.m_domain_name),
            slots
        };
    }

    const Naming& get_naming() const
    {
        return m_naming;
    }

    const SlotReader& get_slot_reader() const
    {
        return m_reader;
    }


private:
    Naming m_naming;
    slot::Reader m_reader;
};

} // namespace cc_abi_gen::parser::vtable
