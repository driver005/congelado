// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/module.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_generator_sonic:module_;

import std;
import :function;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFGeneratorModuleOps : public ice::sonic::Runtime<::TFGeneratorModuleOps, ::TFGeneratorModule>
{
public:
    TFGeneratorModuleOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFGeneratorModuleOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFGeneratorModule* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFGeneratorModuleOps(const ::TFGeneratorModuleOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFGeneratorModuleOps(const ::TFGeneratorModuleOps* ops, ::TFGeneratorModule* handle) noexcept :
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

    void add_function(
        const ice::sonic::TFGeneratorFunctionOps& function,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->add_function(get_handle(), function.get_handle(), out_status.get_handle());
    }

    void get_function(
        const ice::sonic::String& name,
        const ice::sonic::TFGeneratorFunctionOps& out_function,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_function(
            get_handle(),
            name.get_handle(),
            out_function.get_handle(),
            out_status.get_handle()
        );
    }

    void list_functions(
        TF_Tensor** out_functions,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_functions(get_handle(), out_functions, out_status.get_handle());
    }

    void set_name(const ice::sonic::String& name) const noexcept
    {
        m_ops->set_name(get_handle(), name.get_handle());
    }

    void validate(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->validate(get_handle(), out_status.get_handle());
    }

    void emit(
        const ice::sonic::String& out_code,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->emit(get_handle(), out_code.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
