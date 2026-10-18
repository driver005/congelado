// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hash.h"
#include "include/c/intern/registration.h"

export module cc_ice_intern_sonic:hash;

import std;
import :registration;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class Hash : public ice::sonic::Runtime<::TF_HashOps, ::TF_Hash>
{
public:
    Hash(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    Hash(
        const ice::sonic::Registration& registry,
        const ::TF_Hash& handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(
            *registry.get_ops(),
            *registry.get_handle(),
            handle,
            *type.get_handle(),
            *provider.get_handle()
        )
    {
    }

    explicit Hash(const ::TF_HashOps* ops) noexcept :
        Runtime(ops)
    {
    }

    Hash(const ::TF_HashOps* ops, const ::TF_Hash* handle) noexcept :
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

    void hash_bytes(
        const void* data,
        size_t size,
        std::size_t& out_hash,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->hash_bytes(get_handle(), data, size, &out_hash, out_status.get_handle());
    }

    void hash_combine(
        size_t seed,
        size_t value,
        std::size_t& out_hash,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->hash_combine(get_handle(), seed, value, &out_hash, out_status.get_handle());
    }
};

} // namespace ice::sonic
