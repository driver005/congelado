// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/set.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/set.h"

export module cc_abi_sonic_intern;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_SetOps : public ice::sonic::Runtime<TF_SetOps, TF_SetOps>
{
public:
    explicit TF_SetOps(TF_SetOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::Status> insert(const void* key) noexcept
    {
        ice::Status status;
        m_ops->insert(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    find(const void* key, const void** out_value) noexcept
    {
        ice::Status status;
        m_ops->find(get_handle(), key, out_value, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status>
    contains(const void* key, int* out_found) noexcept
    {
        ice::Status status;
        m_ops->contains(get_handle(), key, out_found, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> size(size_t* out_size) noexcept
    {
        ice::Status status;
        m_ops->size(get_handle(), out_size, status.get_handle());

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
