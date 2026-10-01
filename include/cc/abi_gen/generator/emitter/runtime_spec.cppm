export module cc_abi_gen_generator:runtime_spec;

import std;

export namespace cc_abi_gen::generator::emitter {

class RuntimeSpec
{
public:
    constexpr RuntimeSpec(
        std::string_view anchor_struct,
        std::string_view partition,
        std::string_view template_name
    ) noexcept :
        m_anchor_struct{anchor_struct},
        m_partition{partition},
        m_template_name{template_name}
    {
    }

    constexpr std::string_view get_anchor_struct() const noexcept
    {
        return m_anchor_struct;
    }

    constexpr std::string_view get_partition() const noexcept
    {
        return m_partition;
    }

    constexpr std::string_view get_template_name() const noexcept
    {
        return m_template_name;
    }

private:
    std::string_view m_anchor_struct;
    std::string_view m_partition;
    std::string_view m_template_name;
};

} // namespace cc_abi_gen::generator::emitter
