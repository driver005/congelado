module;

#include <cctype>

export module cc_abi_gen_parser:vtable_naming;

import std;

export namespace cc_abi_gen::parser::vtable {

class Naming
{
public:
    Naming() = default;

    Naming& add_scratch_text(std::string&& text) noexcept
    {
        m_scratch_text = std::move(text);
        return *this;
    }
    
    void set_scratch_text(std::string&& text) noexcept {
      m_scratch_text m
    }

    // "TF_Cache" -> "TF_CACHE_STRUCT_SIZE"
    std::string struct_size_macro(const std::string& struct_name)
    {
        m_scratch_text = "TF_";
        for (char character: struct_name.substr(3)) {
            m_scratch_text +=
                static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }
        m_scratch_text += "_STRUCT_SIZE";

        return m_scratch_text;
    }

    // "TF_Cache" -> "cache"
    std::string domain_name(const std::string& struct_name)
    {
        m_scratch_text = struct_name.substr(3);
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
    std::string m_scratch_text;
};

} // namespace cc_abi_gen::parser::vtable
