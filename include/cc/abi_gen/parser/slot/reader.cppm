module;

#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>

export module cc_abi_gen_parser:slot_reader;

import std;
import :helper_parameter;
import :slot_slot;

export namespace cc_abi_gen::parser::slot {

class Reader
{
public:
    Reader() = default;

    Slot read(clang::FieldDecl* field)
    {
        std::vector<helper::Parameter> parameters;

        // Gets the obj representation and casts it to a function type, so we can inspect it.
        auto* function_type =
            field->getType()->getPointeeType()->castAs<clang::FunctionProtoType>();

        auto function_loc = resolve_function_loc(field);

        // Get the number of parameters.
        unsigned parameter_count = function_type->getNumParams();

        // Iterates over the parameters of the function.
        for (unsigned index = 0; index < parameter_count; ++index) {
            // Gets the data type of the parameter at the requested index.
            clang::QualType parameter_type = function_type->getParamType(index);

            parameters.emplace_back(
                helper::Parameter{
                    parameter_type.getAsString(),
                    resolve_pointee_name(parameter_type),
                    resolve_parameter_name(function_loc, index)
                }
            );
        }

        return Slot{
            // Gets the name of the field.
            field->getNameAsString(),
            // Gets the return type of the function.
            function_type->getReturnType().getAsString(),
            std::move(parameters)
        };
    }

private:
    // Bare pointee type name, read structurally off clang's QualType instead of guessed back
    // out of a formatted type string. Empty for void* and for non-pointer parameters.
    std::string resolve_pointee_name(clang::QualType parameter_type)
    {
        if (!parameter_type->isPointerType()) {
            return {};
        }

        clang::QualType pointee_type = parameter_type->getPointeeType();
        if (pointee_type->isVoidType()) {
            return {};
        }

        return pointee_type.getUnqualifiedType().getAsString();
    }

    clang::FunctionProtoTypeLoc resolve_function_loc(clang::FieldDecl* field)
    {
        // Gets the object containing the exact physical locations of the type's tokens in the
        // original source code.
        // NOTE: Defines a Token, as the smallest meaningful building block of source code such as
        // keywords, identifiers, literals, operators, and punctuation.
        clang::TypeSourceInfo* type_source_info = field->getTypeSourceInfo();
        if (type_source_info == nullptr) {
            return clang::FunctionProtoTypeLoc{};
        }

        // Extracts the type's source location, bypass transparent syntax (e.g. parentheses), and
        // cast it to a pointer location.
        auto pointer_loc = type_source_info->getTypeLoc().getAsAdjusted<clang::PointerTypeLoc>();
        if (pointer_loc.isNull()) {
            return clang::FunctionProtoTypeLoc{};
        }

        // Retrieve the source location of the targets, bypass transparent syntax (e.g.
        // parentheses), cast it into a function prototype location, returning null if it is
        // not a function.
        return pointer_loc.getPointeeLoc().getAsAdjusted<clang::FunctionProtoTypeLoc>();
    }

    std::string resolve_parameter_name(clang::FunctionProtoTypeLoc function_loc, unsigned index)
    {
        // If the function location is valid and the index is within the range of parameters.
        if (!function_loc.isNull() && index < function_loc.getNumParams()) {
            // Gets the parameter declaration at the requested index.
            if (clang::ParmVarDecl* parameter_decl = function_loc.getParam(index)) {
                // Gets the name of the parameter.
                std::string name = parameter_decl->getNameAsString();
                if (!name.empty()) {
                    return name;
                }
            }
        }

        // If the parameter name is not found, generate a default name.
        return "arg" + std::to_string(index);
    }
};

} // namespace cc_abi_gen::parser::slot
