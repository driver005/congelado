module;

#include <cctype>

export module cc_abi_gen_parser:vtable_naming;

import std;

export namespace cc_abi_gen::parser::vtable {

// Ops-vtable structs are tagged "TF_XOps" (their handle owns the bare "TF_X" name instead,
// since C++ auto-injects a completed struct's tag as an ordinary type name — the vtable and its
// handle can't both be spelled "TF_X"). Domain/macro/class derivation wants bare "TF_X".
std::string_view strip_ops_suffix(std::string_view struct_name)
{
    static constexpr std::string_view suffix = "Ops";
    if (struct_name.size() > suffix.size() && struct_name.ends_with(suffix)) {
        struct_name.remove_suffix(suffix.size());
    }

    return struct_name;
}

// "TF_Cache" -> "TF_CACHE_STRUCT_SIZE"; "TF_CacheOps" -> "TF_CACHE_STRUCT_SIZE"
std::string struct_size_macro(std::string_view struct_name)
{
    std::string macro = "TF_";
    for (char character: strip_ops_suffix(struct_name).substr(3)) {
        macro += static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
    }
    macro += "_STRUCT_SIZE";

    return macro;
}

// "TF_CacheOps" -> "Cache"; "TF_StringOps" -> "String"; "TFParserDefinitionOps" -> "ParserDefinition"
std::string class_name(std::string_view struct_name)
{
    std::string_view bare = strip_ops_suffix(struct_name);
    if (bare.starts_with("TF_")) {
        bare.remove_prefix(3);
    } else if (bare.starts_with("TF")) {
        bare.remove_prefix(2);
    }

    std::string name{bare};
    if (!name.empty()) {
        name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
    }

    return name;
}

// "TF_PoolId" -> "PoolId"; "TFDataTypeEnum" -> "DataTypeEnum"; "TF_Code" -> "Code"
std::string value_class_name(std::string_view c_name)
{
    std::string_view bare = c_name;
    if (bare.starts_with("TF_")) {
        bare.remove_prefix(3);
    } else if (bare.starts_with("TF")) {
        bare.remove_prefix(2);
    }
    std::string name{bare};
    if (!name.empty()) {
        name[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(name[0])));
    }

    return name;
}

} // namespace cc_abi_gen::parser::vtable
