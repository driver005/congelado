export module cc_abi_gen_parser:value_field;

import std;

export namespace cc_abi_gen::parser::value {

// One data member of a plain C struct. Types are spelled the way generated C++ must write them.
class Field
{
public:
    Field(std::string&& name, std::string&& type, std::size_t extent, bool record) :
        m_name(std::move(name)),
        m_type(std::move(type)),
        m_extent(extent),
        m_record(record)
    {
    }

    ~Field() = default;
    Field(const Field&) = delete;
    Field& operator=(const Field&) = delete;
    Field(Field&&) = default;
    Field& operator=(Field&&) = default;

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
    std::string m_name;
    std::string m_type;
    std::size_t m_extent{0};
    bool m_record{false};
};

} // namespace cc_abi_gen::parser::value
