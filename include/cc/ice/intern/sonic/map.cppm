// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/map.h"

export module cc_ice_intern_sonic:map;

import std;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class TF_MapOps : public ice::sonic::Runtime<::TF_MapOps, ::TF_Map>
{
public:
    template<typename Registry>
    TF_MapOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TF_MapOps(
        Registry& registry,
        ::TF_Map* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TF_MapOps(const ::TF_MapOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_MapOps(const ::TF_MapOps* ops, ::TF_Map* handle) noexcept :
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

    void insert(
        const void* key,
        const void* value,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->insert(get_handle(), key, value, out_status.get_handle());
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

    void for_each(TF_MapVisitor visitor, void* capture) const noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }
};

} // namespace ice::sonic
