// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"

export module cc_ice_extern_generator_sonic:attribute;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorAttributeOps :
    public ice::sonic::Runtime<::TFGeneratorAttributeOps, ::TFGeneratorAttribute>
{
public:
    template<typename Registry>
    TFGeneratorAttributeOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFGeneratorAttributeOps(
        Registry& registry,
        ::TFGeneratorAttribute* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFGeneratorAttributeOps(const ::TFGeneratorAttributeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorAttributeOps(
        const ::TFGeneratorAttributeOps* ops,
        ::TFGeneratorAttribute* handle
    ) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void set_name(const ice::sonic::String& name) const noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }

    void set_description(const ice::sonic::String& description) const noexcept
    {
        m_ops->set_description(get_handle(), description.get_handle());
    }

    void set_full_type(const ice::sonic::String& full_type) const noexcept
    {
        m_ops->set_full_type(get_handle(), full_type.get_handle());
    }

    void set_base_type(const ice::sonic::String& base_type) const noexcept
    {
        m_ops->set_base_type(get_handle(), base_type.get_handle());
    }

    void set_is_list(_Bool is_list) const noexcept
    {
        m_ops->set_is_list(get_handle(), is_list);
    }

    void get_description(const ice::sonic::String& out_description) const noexcept
    {
        m_ops->get_description(get_handle(), out_description.get_handle());
    }

    void get_full_type(const ice::sonic::String& out_full_type) const noexcept
    {
        m_ops->get_full_type(get_handle(), out_full_type.get_handle());
    }

    void get_base_type(const ice::sonic::String& out_base_type) const noexcept
    {
        m_ops->get_base_type(get_handle(), out_base_type.get_handle());
    }

    void is_list(int* out_is_list) const noexcept
    {
        m_ops->is_list(get_handle(), out_is_list);
    }
};

} // namespace ice::sonic
