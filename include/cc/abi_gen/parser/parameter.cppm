export module cc_abi_gen_parser:parameter;

import std;

export namespace cc_abi_gen {

// One function-pointer field parameter, as spelled in the C header's vtable field declarator.
class Parameter
{
public:
    std::string m_type;
    std::string m_name;
};

} // namespace cc_abi_gen
