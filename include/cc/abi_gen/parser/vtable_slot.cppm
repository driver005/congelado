export module cc_abi_gen_parser:vtable_slot;

import std;
import :parameter;

export namespace cc_abi_gen {

// One function-pointer field of a TF_<Name> vtable struct (excluding the leading struct_size).
class VtableSlot
{
public:
    std::string m_name;
    std::string m_return_type;
    std::vector<Parameter> m_parameters;
};

} // namespace cc_abi_gen
