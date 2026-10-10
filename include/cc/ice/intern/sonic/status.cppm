// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"

export module cc_ice_intern_sonic:status;

import std;
import :runtime;
import :tstring;

export namespace ice::sonic {

class Status : public ice::sonic::Runtime<::TF_StatusOps, ::TF_Status>
{
public:
    Status(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    Status(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Status* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit Status(const ::TF_StatusOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Status(const ::TF_StatusOps* ops, ::TF_Status* handle) noexcept :
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

    void delete_status() const noexcept
    {
        m_ops->delete_status(get_handle());
    }

    void set_status(TF_Code code, const ice::sonic::String& msg) const noexcept
    {
        m_ops->set_status(get_handle(), code, msg.get_handle());
    }

    void set_payload(const ice::sonic::String& key, const ice::sonic::String& value) const noexcept
    {
        m_ops->set_payload(get_handle(), key.get_handle(), value.get_handle());
    }

    void for_each_payload(TF_PayloadVisitor visitor, void* capture) const noexcept
    {
        m_ops->for_each_payload(get_handle(), visitor, capture);
    }

    void set_status_from_io_error(int error_code, const ice::sonic::String& context) const noexcept
    {
        m_ops->set_status_from_io_error(get_handle(), error_code, context.get_handle());
    }

    void get_code(TF_Code* out_code) const noexcept
    {
        m_ops->get_code(get_handle(), out_code);
    }

    void message(const ice::sonic::String& out_message) const noexcept
    {
        m_ops->message(get_handle(), out_message.get_handle());
    }
};

} // namespace ice::sonic
