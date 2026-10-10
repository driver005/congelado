// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/deque.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/deque.h"

export module cc_ice_intern_sonic:deque;

import std;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class TF_DequeOps : public ice::sonic::Runtime<::TF_DequeOps, ::TF_Deque>
{
public:
    TF_DequeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_DequeOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Deque* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_DequeOps(const ::TF_DequeOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_DequeOps(const ::TF_DequeOps* ops, ::TF_Deque* handle) noexcept :
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

    void set_element_size(size_t element_size) const noexcept
    {
        m_ops->set_element_size(get_handle(), element_size);
    }

    void push_front(const void* value) const noexcept
    {
        m_ops->push_front(get_handle(), value);
    }

    void push_back(const void* value) const noexcept
    {
        m_ops->push_back(get_handle(), value);
    }

    void pop_front() const noexcept
    {
        m_ops->pop_front(get_handle());
    }

    void pop_back() const noexcept
    {
        m_ops->pop_back(get_handle());
    }

    void get(
        size_t index,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get(get_handle(), index, out_value, out_status.get_handle());
    }

    void size(size_t* out_size) const noexcept
    {
        m_ops->size(get_handle(), out_size);
    }
};

} // namespace ice::sonic
