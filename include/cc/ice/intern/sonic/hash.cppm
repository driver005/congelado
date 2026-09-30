// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hash.h"

export module cc_ice_intern_sonic:hash;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_HashOps : public ice::sonic::Runtime<TF_HashOps, TF_HashOps>
{
public:
    explicit TF_HashOps(TF_HashOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    hash_bytes(const void* data, size_t size, size_t* out_hash) noexcept
    {
        ice::sonic::Status status;
        m_ops->hash_bytes(get_handle(), data, size, out_hash, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    hash_combine(size_t seed, size_t value, size_t* out_hash) noexcept
    {
        ice::sonic::Status status;
        m_ops->hash_combine(get_handle(), seed, value, out_hash, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
