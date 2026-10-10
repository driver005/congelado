// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/parser/module.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/parser/module.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_parser_sonic:module_;

import std;
import :function;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFParserModuleOps : public ice::sonic::Runtime<::TFParserModuleOps, ::TFParserModule>
{
public:
    TFParserModuleOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFParserModuleOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFParserModule* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFParserModuleOps(const ::TFParserModuleOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFParserModuleOps(const ::TFParserModuleOps* ops, ::TFParserModule* handle) noexcept :
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

    void get_name(
        const ice::sonic::String& out_name,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle(), out_status.get_handle());
    }

    void get_function_count(int* out_count, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->get_function_count(get_handle(), out_count, out_status.get_handle());
    }

    void get_function(
        int index,
        const ice::sonic::TFParserFunctionOps& out_function,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops
            ->get_function(get_handle(), index, out_function.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
