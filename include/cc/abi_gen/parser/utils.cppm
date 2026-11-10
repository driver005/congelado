module;

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>

export module cc_abi_gen_parser:utils;

import std;
import cc_abi_gen_classify;

export namespace cc_abi_gen::parser {

// Example: "TFGrapplerConfigsOps" -> "TFGrapplerConfigs" when a slot takes that handle
std::string c_handle_name(const classify::Classify& model)
{
    std::string_view struct_name = model.get_c_name();
    if (!struct_name.ends_with("Ops")) {
        return {};
    }

    std::string handle_name{struct_name.substr(0, struct_name.size() - 3)};
    for (const classify::Function& slot: model.get_functions()) {
        auto parameters = slot.get_parameters();
        if (!parameters.empty() && parameters.front().get_pointee_name() == handle_name) {
            return handle_name;
        }
    }

    return {};
}

// Non-const pointer to a number, bool or enum (char excluded).
bool is_scalar_pointer(clang::QualType parameter_type) noexcept
{
    if (!parameter_type->isPointerType()) {
        return false;
    }

    const clang::QualType POINTEE_TYPE = parameter_type->getPointeeType();
    if (POINTEE_TYPE.isConstQualified() || POINTEE_TYPE->isAnyCharacterType()) {
        return false;
    }

    return POINTEE_TYPE->isArithmeticType() || POINTEE_TYPE->isEnumeralType();
}

// Pointer to numbers or enums; char and bool stay out (strings, no vector<bool>::data).
bool is_element_pointer(clang::QualType parameter_type) noexcept
{
    if (!parameter_type->isPointerType()) {
        return false;
    }

    const clang::QualType POINTEE_TYPE = parameter_type->getPointeeType();
    if (POINTEE_TYPE->isAnyCharacterType() || POINTEE_TYPE->isBooleanType()) {
        return false;
    }

    return POINTEE_TYPE->isArithmeticType() || POINTEE_TYPE->isEnumeralType();
}

// Bare pointee type name, read off clang's QualType. Empty for void* and non-pointers.
std::string resolve_pointee_name(clang::QualType parameter_type)
{
    if (!parameter_type->isPointerType() || parameter_type->getPointeeType()->isVoidType()) {
        return {};
    }

    return parameter_type->getPointeeType().getUnqualifiedType().getAsString();
}

// Written declarator of the slot, the only place that still carries parameter names.
clang::FunctionProtoTypeLoc resolve_function_loc(clang::FieldDecl* field) noexcept
{
    clang::TypeSourceInfo* type_source_info = field->getTypeSourceInfo();
    if (type_source_info == nullptr) {
        return clang::FunctionProtoTypeLoc{};
    }

    auto pointer_loc = type_source_info->getTypeLoc().getAsAdjusted<clang::PointerTypeLoc>();
    if (pointer_loc.isNull()) {
        return clang::FunctionProtoTypeLoc{};
    }

    return pointer_loc.getPointeeLoc().getAsAdjusted<clang::FunctionProtoTypeLoc>();
}

// Example: "status", or "arg2" when the header leaves the parameter unnamed.
std::string resolve_parameter_name(clang::FunctionProtoTypeLoc function_loc, unsigned index)
{
    if (!function_loc.isNull() && index < function_loc.getNumParams()) {
        if (clang::ParmVarDecl* parameter_decl = function_loc.getParam(index)) {
            std::string name = parameter_decl->getNameAsString();
            if (!name.empty()) {
                return name;
            }
        }
    }

    return "arg" + std::to_string(index);
}

// One function pointer field becomes one function; the first parameter is the handle.
std::optional<classify::Function> read_slot(clang::FieldDecl* field)
{
    const auto* function_type =
        field->getType()->getPointeeType()->getAs<clang::FunctionProtoType>();
    if (function_type == nullptr) {
        return std::nullopt;
    }

    const auto FUNCTION_LOC = resolve_function_loc(field);

    classify::Function slot{field->getNameAsString(), function_type->getReturnType().getAsString()};
    for (unsigned index = 0; index < function_type->getNumParams(); ++index) {
        const clang::QualType PARAMETER_TYPE = function_type->getParamType(index);

        classify::Param parameter{
            PARAMETER_TYPE.getAsString(),
            resolve_parameter_name(FUNCTION_LOC, index)
        };
        parameter.add_pointee_name(resolve_pointee_name(PARAMETER_TYPE))
            .add_scalar_pointer(is_scalar_pointer(PARAMETER_TYPE))
            .add_element_pointer(is_element_pointer(PARAMETER_TYPE));
        slot.set_param(std::move(parameter));
    }

    return slot;
}

// `type name() [const] noexcept { body }`
classify::Function getter(std::string name, std::string type, std::string body, bool is_const)
{
    classify::Function function{std::move(name), std::move(type)};
    function.set_const(is_const).set_noexcept().set_body_line(std::move(body));

    return function;
}

// Element count for fixed size arrays, 0 for everything else.
std::size_t field_extent(const clang::FieldDecl* field)
{
    if (const auto* array = field->getASTContext().getAsConstantArrayType(field->getType())) {
        return static_cast<std::size_t>(array->getSize().getZExtValue());
    }

    return 0;
}

// Field type, the element type for arrays.
clang::QualType element_qual_type(const clang::FieldDecl* field)
{
    if (const auto* array = field->getASTContext().getAsConstantArrayType(field->getType())) {
        return array->getElementType();
    }

    return field->getType();
}

// Writes global names with `::` and fixed width integers with `std::`.
std::string spell(const std::string& type)
{
    static constexpr std::array STD_NAMES{
        "int8_t",
        "int16_t",
        "int32_t",
        "int64_t",
        "uint8_t",
        "uint16_t",
        "uint32_t",
        "uint64_t",
        "intptr_t",
        "uintptr_t",
        "size_t",
        "ptrdiff_t",
    };
    static constexpr std::array KEYWORDS{"const", "struct", "enum", "union", "volatile"};

    auto is_identifier = [](unsigned char token)
    {
        return std::isalnum(token) || token == '_';
    };

    std::string result;
    for (auto chunk: type | std::views::chunk_by(
                                [is_identifier](char left, char right)
                                {
                                    return is_identifier(left) == is_identifier(right);
                                }
                            )) {
        std::string_view token{chunk.begin(), chunk.end()};
        if (is_identifier(token.front())) {
            if (std::ranges::contains(STD_NAMES, token)) {
                result += "std::";
            } else if (!std::ranges::contains(KEYWORDS, token) && token.starts_with("TF")) {
                result += "::";
            }
        }
        result += token;
    }

    return result;
}

// Example: std::uint8_t, ::TF_PoolId, const char*
std::string field_type(const clang::FieldDecl* field)
{
    return spell(element_qual_type(field).getAsString());
}

// Struct and union members are handed out by reference.
bool is_record(const clang::FieldDecl* field)
{
    return element_qual_type(field)->isRecordType();
}

// field_name -> FieldName
std::string camel(std::string_view name)
{
    std::string result;
    bool upper = true;
    for (const char CHARACTER: name) {
        if (CHARACTER == '_') {
            upper = true;
            continue;
        }
        result += upper ? static_cast<char>(std::toupper(static_cast<unsigned char>(CHARACTER)))
                        : CHARACTER;
        upper = false;
    }

    return result;
}

// Array: span getters. Record: reference getters. Anything else: value getter and setter.
void add_field_accessors(classify::Classify& cls, const clang::FieldDecl* field)
{
    const std::string NAME = field->getNameAsString();
    const std::string ACCESSOR = camel(NAME);
    const std::string TYPE = field_type(field);
    const std::string READ = std::format("return m_value.{};", NAME);

    if (const std::size_t EXTENT = field_extent(field); EXTENT != 0) {
        cls.set_function(
            getter("get" + ACCESSOR, std::format("std::span<{}, {}>", TYPE, EXTENT), READ, false)
        );
        cls.set_function(getter(
            "get" + ACCESSOR,
            std::format("std::span<const {}, {}>", TYPE, EXTENT),
            READ,
            true
        ));
        return;
    }

    if (is_record(field)) {
        cls.set_function(getter("get" + ACCESSOR, TYPE + "&", READ, false));
        cls.set_function(getter("get" + ACCESSOR, std::format("const {}&", TYPE), READ, true));
        return;
    }

    cls.set_function(getter("get" + ACCESSOR, TYPE, READ, true));
    cls.set_function(
        classify::Function{"set" + ACCESSOR, "void"}
            .set_noexcept()
            .set_param(TYPE, "value")
            .set_body_line(std::format("m_value.{} = value;", NAME))
    );
}

// Prefix every enumerator shares, up to its last underscore: TF_MEMORY_SPACE_.
std::size_t common_prefix(std::span<const std::string> names)
{
    if (names.empty()) {
        return 0;
    }

    std::size_t common = names.front().size();
    for (const std::string& name: names) {
        std::size_t index = 0;
        while (index < common && index < name.size() && name.at(index) == names.front().at(index)) {
            ++index;
        }
        common = index;
    }

    const auto UNDERSCORE = names.front().rfind('_', common == 0 ? 0 : common - 1);

    return UNDERSCORE == std::string::npos || common == 0 ? 0 : UNDERSCORE + 1;
}

// `enum Value : std::underlying_type_t<::TF_Code> { k_ok = ::TF_OK, … };`
std::string value_enum(clang::EnumDecl* enum_decl, std::string_view c_name)
{
    std::vector<std::string> names;
    for (clang::EnumConstantDecl* constant: enum_decl->enumerators()) {
        names.push_back(constant->getNameAsString());
    }

    const std::size_t COMMON = common_prefix(names);

    std::string text = std::format("enum Value : std::underlying_type_t<::{}>\n{{\n", c_name);
    for (std::size_t index = 0; index < names.size(); ++index) {
        const std::string& name = names.at(index);
        std::string rest = name.substr(std::min(COMMON, name.size() - 1));
        for (char& character: rest) {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        text +=
            std::format("    k_{} = ::{}{}\n", rest, name, index + 1 == names.size() ? "" : ",");
    }
    text += "};\n";

    return text;
}

classify::Function constructor(const std::string& name)
{
    classify::Function function{name, "", classify::FunctionType::CONSTRUCTOR};
    function.set_noexcept();

    return function;
}

// Slot parameters after the leading handle.
std::span<const classify::Param> slot_arguments(const classify::Function& slot) noexcept
{
    auto parameters = slot.get_parameters();

    return parameters.empty() ? parameters : parameters.subspan(1);
}

std::string_view self_parameter_name(const classify::Function& slot) noexcept
{
    auto parameters = slot.get_parameters();

    return parameters.empty() ? std::string_view{"plugin_context"}
                              : std::string_view{parameters.front().get_name()};
}

bool is_length_name(std::string_view name) noexcept
{
    static constexpr std::array<std::string_view, 5>
        LENGTH_WORDS{"num", "count", "capacity", "max", "len"};

    return std::ranges::any_of(
        LENGTH_WORDS,
        [name](std::string_view word)
        {
            return name.contains(word);
        }
    );
}

// A slot with a length or capacity parameter hands out arrays, which stay plain pointers.
bool has_array(const classify::Function& slot) noexcept
{
    return std::ranges::any_of(
        slot_arguments(slot),
        [](const classify::Param& candidate)
        {
            return !candidate.get_type().contains('*') &&
                   !candidate.get_name().starts_with("out_") &&
                   is_length_name(candidate.get_name());
        }
    );
}

// The integer parameter that carries the element count of the array starting at `parameter`.
const classify::Param* length_of(const classify::Function& slot, const classify::Param& parameter)
{
    if (!parameter.is_element_pointer()) {
        return nullptr;
    }

    const auto PARAMETERS = slot_arguments(slot);
    const auto INDEX = static_cast<std::size_t>(&parameter - PARAMETERS.data());
    // A string list hands out char** next to its lengths; those stay plain pointers.
    if (INDEX > 0 && PARAMETERS.at(INDEX - 1).get_type().ends_with("**")) {
        return nullptr;
    }

    if (INDEX + 1 < PARAMETERS.size()) {
        const classify::Param& candidate = PARAMETERS.at(INDEX + 1);
        if (candidate.is_integer() && !candidate.get_name().starts_with("out_") &&
            is_length_name(candidate.get_name())) {
            return &candidate;
        }
    }

    // An array right behind a length shares it: tensor_view(dims, num_dims, strides, ...).
    if (INDEX > 1 && PARAMETERS.at(INDEX - 1).is_integer() &&
        length_of(slot, PARAMETERS.at(INDEX - 2)) == &PARAMETERS.at(INDEX - 1)) {
        return &PARAMETERS.at(INDEX - 1);
    }

    return nullptr;
}

bool is_array_length(const classify::Function& slot, const classify::Param& parameter)
{
    const auto PARAMETERS = slot_arguments(slot);
    const auto INDEX = static_cast<std::size_t>(&parameter - PARAMETERS.data());

    return INDEX > 0 && length_of(slot, PARAMETERS.at(INDEX - 1)) == &parameter;
}

// The sonic argument for an element count: the smallest size of the vectors that share it.
std::string length_argument(const classify::Function& slot, const classify::Param& length)
{
    std::string size;
    std::size_t count = 0;
    for (const classify::Param& candidate: slot_arguments(slot)) {
        if (length_of(slot, candidate) == &length) {
            size += std::format("{}{}.size()", count++ == 0 ? "" : ", ", candidate.get_name());
        }
    }

    return std::format(
        "static_cast<{}>({})",
        length.get_type(),
        count > 1 ? std::format("std::min({{{}}})", size) : size
    );
}

bool is_named_definition(const clang::TagDecl* decl)
{
    return decl->isCompleteDefinition() && !decl->getNameAsString().empty();
}

// Members of unnamed struct or union type have no spellable C++ name.
bool has_anonymous_type(clang::QualType type)
{
    const std::string SPELLING = type.getAsString();

    return SPELLING.contains("unnamed") || SPELLING.contains("anonymous");
}

// "TF_CacheOps" -> "Cache"; "TFParserDefinitionOps" -> "ParserDefinition"; "TF_Code" -> "Code"
std::string class_name(std::string_view c_name)
{
    if (c_name.size() > 3 && c_name.ends_with("Ops")) {
        c_name.remove_suffix(3);
    }
    if (c_name.starts_with("TF_")) {
        c_name.remove_prefix(3);
    } else if (c_name.starts_with("TF")) {
        c_name.remove_prefix(2);
    }

    std::string name{c_name};
    if (!name.empty()) {
        name.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(name.front())));
    }

    return name;
}

} // namespace cc_abi_gen::parser
