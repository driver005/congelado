// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/set.h"

export module cc_ice_intern_sonic:set;

import std;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class TF_SetOps : public ice::sonic::Runtime<::TF_SetOps, ::TF_Set>
{
public:
    template<typename Registry>
    TF_SetOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_SetOps(
        Registry& registry,
        ::TF_Set* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_SetOps(const ::TF_SetOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_SetOps(const ::TF_SetOps* ops, ::TF_Set* handle) noexcept :
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

    void insert(const void* key, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->insert(get_handle(), key, out_status.get_handle());
    }

    void find(
        const void* key,
        const void** out_value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->find(get_handle(), key, out_value, out_status.get_handle());
    }

    void erase(const void* key, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->erase(get_handle(), key, out_status.get_handle());
    }

    void contains(
        const void* key,
        int* out_found,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->contains(get_handle(), key, out_found, out_status.get_handle());
    }

    void size(size_t* out_size) const noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void for_each(TF_SetVisitor visitor, void* capture) const noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }
};

} // namespace ice::sonic
