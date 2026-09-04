module;

#include <clang/AST/Decl.h>
#include <clang/AST/RecursiveASTVisitor.h>

export module cc_abi_gen_parser:vtable_ast_visitor;

import std;
import :vtable_slot;
import :vtable_model;
import :vtable_naming;
import :slot_reader;

export namespace cc_abi_gen {

// Walks one parsed header looking for the one vtable struct: a complete RecordDecl whose first
// field is named struct_size. Structural, not name-based, so it generalizes to every other
// domain without hardcoding TF_<Name>.
class VtableAstVisitor : public clang::RecursiveASTVisitor<VtableAstVisitor>
{
public:
    bool VisitRecordDecl(clang::RecordDecl *record_decl)
    {
        if (!record_decl->isCompleteDefinition()) {

            return true;
        }

        auto field_iterator = record_decl->field_begin();
        if (field_iterator == record_decl->field_end()) {

            return true;
        }

        clang::FieldDecl *first_field = *field_iterator;
        if (first_field->getNameAsString() != "struct_size") {

            return true;
        }

        VtableModel model;
        model.m_struct_name = record_decl->getNameAsString();
        model.m_struct_size_macro = m_naming.struct_size_macro(model.m_struct_name);
        model.m_domain_name = m_naming.domain_name(model.m_struct_name);
        model.m_class_name = m_naming.class_name(model.m_domain_name);

        m_scratch_slots.clear();
        for (++field_iterator; field_iterator != record_decl->field_end(); ++field_iterator) {

            clang::FieldDecl *field = *field_iterator;
            if (!field->getType()->isFunctionPointerType()) {

                continue;
            }

            m_scratch_slots.push_back(m_slot_reader.read(field));
        }

        model.m_slots = m_scratch_slots;
        m_model = std::move(model);

        return true;
    }

    std::optional<VtableModel> m_model;

private:
    VtableNaming m_naming;
    SlotReader m_slot_reader;
    std::vector<VtableSlot> m_scratch_slots;
};

} // namespace cc_abi_gen
