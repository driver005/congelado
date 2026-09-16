// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/hash/hash.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/hash/hash.h"

export module cc_abi_sonic_hash;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Hash : public ice::sonic::Runtime<Hash, TF_Hash>
{
public:
    explicit Hash(TF_Hash* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "hash";

    [[nodiscard]] std::expected<void, ice::Status>
    hash_bytes(const void* data, size_t size) noexcept
    {
        ice::Status status;
        m_ops->hash_bytes(get_handle(), data, size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> hash_combine(size_t seed, size_t value) noexcept
    {
        ice::Status status;
        m_ops->hash_combine(get_handle(), seed, value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
