// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/parameter.h"

export module cc_ice_extern_generator_sonic:parameter;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorParameterOps :
    public ice::sonic::Runtime<TFGeneratorParameterOps, TFGeneratorParameterOps>
{
public:
    explicit TFGeneratorParameterOps(TFGeneratorParameterOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_name(const ice::sonic::String& name) noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }

    void set_description(const ice::sonic::String& description) noexcept
    {
        m_ops->set_description(get_handle(), description.get_handle());
    }

    void set_position(int position) noexcept
    {
        m_ops->set_position(get_handle(), position);
    }

    void get_description(const ice::sonic::String& out_description) noexcept
    {
        m_ops->get_description(get_handle(), out_description.get_handle());
    }

    void get_position(int* out_position) noexcept
    {
        m_ops->get_position(get_handle(), out_position);
    }

    void get_type(const ice::sonic::TF_TypeInfoOps& out_type) noexcept
    {
        m_ops->get_type(get_handle(), out_type.get_handle());
    }
};

} // namespace ice::sonic
