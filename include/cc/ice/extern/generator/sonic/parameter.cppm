// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/parameter.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/parameter.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_generator_sonic:parameter;

import std;
import :typeinfo;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class GeneratorParameter :
    public ice::sonic::Runtime<::TFGeneratorParameterOps, ::TFGeneratorParameter>
{
public:
    GeneratorParameter(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    GeneratorParameter(
        const ice::sonic::Registration& registry,
        ::TFGeneratorParameter* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            registry.get_handle(),
            handle,
            type.get_handle(),
            provider.get_handle()
        )
    {
    }

    explicit GeneratorParameter(const ::TFGeneratorParameterOps* ops) noexcept :
        Runtime(ops)
    {
    }

    GeneratorParameter(
        const ::TFGeneratorParameterOps* ops,
        ::TFGeneratorParameter* handle
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

    void set_position(int position) const noexcept
    {
        m_ops->set_position(get_handle(), position);
    }

    void get_description(const ice::sonic::String& out_description) const noexcept
    {
        m_ops->get_description(get_handle(), out_description.get_handle());
    }

    void get_position(int* out_position) const noexcept
    {
        m_ops->get_position(get_handle(), out_position);
    }

    void get_type(const ice::sonic::TypeInfo& out_type) const noexcept
    {
        m_ops->get_type(get_handle(), out_type.get_handle());
    }
};

} // namespace ice::sonic
