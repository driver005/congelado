export module cc_abi_gen_parser:vtable_model;

import std;
import :vtable_slot;

export namespace cc_abi_gen {

// The parsed shape of one include/c/extern/<domain>/<domain>.h vtable header.
class VtableModel
{
public:
    std::string m_struct_name;
    std::string m_struct_size_macro;
    std::string m_domain_name;
    std::string m_class_name;
    std::vector<VtableSlot> m_slots;
};

} // namespace cc_abi_gen
