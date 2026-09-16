module;

#include <cctype>

export module cc_abi_gen_parser:vtable_naming;

import std;

export namespace cc_abi_gen::parser::vtable {

class Naming
{
public:
    Naming() = default;

    // "TF_Cache" -> "TF_CACHE_STRUCT_SIZE"; "TF_CacheOps" -> "TF_CACHE_STRUCT_SIZE"
    std::string struct_size_macro(const std::string& struct_name)
    {
        m_scratch_text = "TF_";
        for (char character: strip_ops_suffix(struct_name).substr(3)) {
            m_scratch_text +=
                static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }
        m_scratch_text += "_STRUCT_SIZE";

        return m_scratch_text;
    }

    // "TF_Cache" -> "cache"; "TF_CacheOps" -> "cache"
    std::string domain_name(const std::string& struct_name)
    {
        m_scratch_text = strip_ops_suffix(struct_name).substr(3);
        for (char& character: m_scratch_text) {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }

        return m_scratch_text;
    }

    // "cache" -> "Cache"
    std::string class_name(const std::string& domain)
    {
        m_scratch_text = domain;
        if (!m_scratch_text.empty()) {
            m_scratch_text[0] =
                static_cast<char>(std::toupper(static_cast<unsigned char>(m_scratch_text[0])));
        }

        return m_scratch_text;
    }

private:
    // Ops-vtable structs are tagged "TF_XOps" (their handle owns the bare "TF_X" name instead, since C++ auto-injects a completed struct's tag as an ordinary type name — the vtable and its handle can't both be spelled "TF_X"). Domain/macro derivation still wants bare "TF_X".
    static std::string strip_ops_suffix(const std::string& struct_name)
    {
        static constexpr std::string_view suffix = "Ops";
        if (struct_name.size() > suffix.size() && struct_name.ends_with(suffix)) {
            return struct_name.substr(0, struct_name.size() - suffix.size());
        }
        return struct_name;
    }

    std::string m_scratch_text;
};

} // namespace cc_abi_gen::parser::vtable
