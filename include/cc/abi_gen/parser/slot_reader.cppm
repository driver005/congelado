module;

#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>

export module cc_abi_gen_parser:slot_reader;

import std;
import :parameter;
import :vtable_slot;

export namespace cc_abi_gen {

// Reads one function-pointer field's slot name, return type, and parameter (type, name) pairs.
// Parameter names live on the field's FunctionProtoTypeLoc (the written declarator), not on the
// canonical FunctionProtoType, which is name-erased.
class SlotReader
{
public:
    VtableSlot read(clang::FieldDecl *field)
    {
        m_scratch_parameters.clear();

        VtableSlot slot;
        slot.m_name = field->getNameAsString();

        const auto *function_type =
            field->getType()->getPointeeType()->castAs<clang::FunctionProtoType>();
        slot.m_return_type = function_type->getReturnType().getAsString();

        clang::FunctionProtoTypeLoc function_loc = resolve_function_loc(field);

        unsigned parameter_count = function_type->getNumParams();
        for (unsigned index = 0; index < parameter_count; ++index) {

            Parameter parameter;
            parameter.m_type = function_type->getParamType(index).getAsString();
            parameter.m_name = resolve_parameter_name(function_loc, index);
            m_scratch_parameters.push_back(std::move(parameter));
        }

        slot.m_parameters = m_scratch_parameters;

        return slot;
    }

private:
    clang::FunctionProtoTypeLoc resolve_function_loc(clang::FieldDecl *field)
    {
        clang::TypeSourceInfo *type_source_info = field->getTypeSourceInfo();
        if (type_source_info == nullptr) {

            return clang::FunctionProtoTypeLoc{};
        }

        auto pointer_loc = type_source_info->getTypeLoc().getAsAdjusted<clang::PointerTypeLoc>();
        if (pointer_loc.isNull()) {

            return clang::FunctionProtoTypeLoc{};
        }

        return pointer_loc.getPointeeLoc().getAsAdjusted<clang::FunctionProtoTypeLoc>();
    }

    std::string resolve_parameter_name(clang::FunctionProtoTypeLoc function_loc, unsigned index)
    {
        if (!function_loc.isNull() && index < function_loc.getNumParams()) {

            if (clang::ParmVarDecl *parameter_decl = function_loc.getParam(index)) {

                std::string name = parameter_decl->getNameAsString();
                if (!name.empty()) {

                    return name;
                }
            }
        }

        return "arg" + std::to_string(index);
    }

    std::vector<Parameter> m_scratch_parameters;
};

} // namespace cc_abi_gen
