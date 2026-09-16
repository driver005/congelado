// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/set/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/set/set.h"

export module cc_abi_sonic_set;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Set : public ice::sonic::Runtime<Set, TF_Set>
{
public:
    explicit Set(TF_Set* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "set";

    [[nodiscard]] std::expected<void, ice::Status> new_set(
        size_t key_size,
        TF_SetHashFn hash_fn,
        TF_SetCompareFn compare_fn,
        int allow_duplicates
    ) noexcept
    {
        ice::Status status;
        m_ops->new_set(
            get_handle(),
            key_size,
            hash_fn,
            compare_fn,
            allow_duplicates,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> insert(const void* key) noexcept
    {
        ice::Status status;
        m_ops->insert(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> find(const void* key) noexcept
    {
        ice::Status status;
        m_ops->find(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> erase(const void* key) noexcept
    {
        ice::Status status;
        m_ops->erase(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> contains(const void* key) noexcept
    {
        ice::Status status;
        m_ops->contains(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> size() noexcept
    {
        ice::Status status;
        m_ops->size(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_SetVisitor visitor, void* capture) noexcept
    {
        ice::Status status;
        m_ops->for_each(get_handle(), visitor, capture, status.get_handle());

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
