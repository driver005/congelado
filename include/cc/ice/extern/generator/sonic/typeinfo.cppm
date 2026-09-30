// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/typeinfo.h"

export module cc_ice_extern_generator_sonic:typeinfo;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TypeInfoOps : public ice::sonic::Runtime<TF_TypeInfoOps, TF_TypeInfoOps>
{
public:
    explicit TF_TypeInfoOps(TF_TypeInfoOps* ops, void* plugin_context) noexcept :
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

    void set_type_attr_name(const ice::sonic::String& type_attr_name) noexcept
    {
        m_ops->set_type_attr_name(get_handle(), type_attr_name.get_handle());
    }

    void set_data_type(int data_type) noexcept
    {
        m_ops->set_data_type(get_handle(), data_type);
    }

    void set_read_only(_Bool read_only) noexcept
    {
        m_ops->set_read_only(get_handle(), read_only);
    }

    void set_list(_Bool is_list) noexcept
    {
        m_ops->set_list(get_handle(), is_list);
    }

    void get_type_attr_name(const ice::sonic::String& out_type_attr_name) noexcept
    {
        m_ops->get_type_attr_name(get_handle(), out_type_attr_name.get_handle());
    }

    void get_data_type(int* out_data_type) noexcept
    {
        m_ops->get_data_type(get_handle(), out_data_type);
    }

    void is_read_only(int* out_is_read_only) noexcept
    {
        m_ops->is_read_only(get_handle(), out_is_read_only);
    }

    void is_list(int* out_is_list) noexcept
    {
        m_ops->is_list(get_handle(), out_is_list);
    }
};

} // namespace ice::sonic
