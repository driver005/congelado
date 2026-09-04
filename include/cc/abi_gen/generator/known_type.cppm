export module cc_abi_gen_generator:known_type;

import std;

export namespace cc_abi_gen {

// How one C intern/value type crossing the vtable ABI boundary maps to its C++ wrapper, on both
// sides: m_cpp_parameter_type replaces the raw C parameter type in a builder/sonic method
// signature; m_wrap_format turns a C argument into that C++ value (builder-lambda boundary,
// e.g. "ice::String::create({})"); m_unwrap_format turns the C++ value back into a C-ABI
// argument (sonic-call boundary, e.g. "{}.get_handle()"). Either format may be empty when the
// type is known but never itself needs converting (e.g. TF_Status, always the trailing
// fallibility marker rather than a wrapped parameter).
class KnownType
{
public:
    std::string m_pointee_name;
    std::string m_cpp_parameter_type;
    std::string m_wrap_format;
    std::string m_unwrap_format;
};

} // namespace cc_abi_gen
