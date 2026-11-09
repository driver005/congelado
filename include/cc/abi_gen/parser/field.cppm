module;

#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>

export module cc_abi_gen_parser:field;

import std;

export namespace cc_abi_gen::parser {

// One data member of a plain C struct. Types are spelled the way generated C++ must write them.
class Field
{
public:
    Field() = default;

    Field(std::string&& name, std::string&& type, std::size_t extent, bool record) :
        m_name{std::move(name)},
        m_type{std::move(type)},
        m_extent{extent},
        m_record{record}
    {
    }

    ~Field() = default;
    Field(const Field&) = delete;
    Field& operator=(const Field&) = delete;
    Field(Field&&) = default;
    Field& operator=(Field&&) = default;

    void read_clang(clang::FieldDecl* field)
    {
        m_name = field->getNameAsString();

        clang::QualType type = field->getType();

        std::size_t extent = 0;
        if (const auto* array = field->getASTContext().getAsConstantArrayType(type)) {
            m_extent = static_cast<std::size_t>(array->getSize().getZExtValue());
            type = array->getElementType();
        }

        m_type = spell(type.getAsString());
        m_record = type->isRecordType();
    }

    // Example: max_clock_frequency
    const std::string& get_name() const noexcept
    {
        return m_name;
    }

    // Element type for arrays. Example: std::uint8_t, ::TF_PoolId, const char*
    const std::string& get_type() const noexcept
    {
        return m_type;
    }

    // Element count for fixed size arrays, 0 for everything else.
    std::size_t get_extent() const noexcept
    {
        return m_extent;
    }

    bool is_array() const noexcept
    {
        return m_extent != 0;
    }

    // True for struct and union members, which accessors hand out by reference.
    bool is_record() const noexcept
    {
        return m_record;
    }

private:
    // Writes global names with `::` and fixed width integers with `std::`.
    static std::string spell(const std::string& type)
    {
        static constexpr std::array std_names{
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
            "ptrdiff_t"
        };

        static constexpr std::array keywords{"const", "struct", "enum", "union", "volatile"};

        std::string result;
        auto is_id = [](unsigned char token)
        {
            return std::isalnum(token) || token == '_';
        };

        for (auto chunk: type | std::views::chunk_by(
                                    [is_id](char token_a, char token_b)
                                    {
                                        return is_id(token_a) == is_id(token_b);
                                    }
                                )) {
            std::string_view token{chunk.begin(), chunk.end()};
            if (is_id(token[0])) {
                if (std::ranges::contains(std_names, token)) {
                    result += "std::";
                } else if (!std::ranges::contains(keywords, token) && token.starts_with("TF")) {
                    result += "::";
                }
            }
            result += token;
        }
        return result;
    }

    std::string m_name;
    std::string m_type;
    std::size_t m_extent{0};
    bool m_record{false};
};

} // namespace cc_abi_gen::parser
