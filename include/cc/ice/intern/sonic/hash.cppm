// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/registration/registration.h"
#include "include/c/intern/hash.h"

export module cc_ice_intern_sonic:hash;

import std;
import :runtime;
import :status;
import :tstring;

export namespace ice::sonic {

class TF_HashOps : public ice::sonic::Runtime<::TF_HashOps, ::TF_Hash>
{
public:
    TF_HashOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TF_HashOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TF_Hash* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TF_HashOps(const ::TF_HashOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TF_HashOps(const ::TF_HashOps* ops, ::TF_Hash* handle) noexcept :
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
        size_t* out_hash,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->hash_bytes(get_handle(), data, size, out_hash, out_status.get_handle());
    }

    void hash_combine(
        size_t seed,
        size_t value,
        size_t* out_hash,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->hash_combine(get_handle(), seed, value, out_hash, out_status.get_handle());
    }
};

} // namespace ice::sonic
