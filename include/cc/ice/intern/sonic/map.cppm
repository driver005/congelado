// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/map.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/map.h"

export module cc_ice_intern_sonic:map;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_MapOps : public ice::sonic::Runtime<TF_MapOps, TF_MapOps>
{
public:
    explicit TF_MapOps(TF_MapOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    insert(const void* key, const void* value) noexcept
    {
        ice::sonic::Status status;
        m_ops->insert(get_handle(), key, value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    find(const void* key, const void** out_value) noexcept
    {
        ice::sonic::Status status;
        m_ops->find(get_handle(), key, out_value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> erase(const void* key) noexcept
    {
        ice::sonic::Status status;
        m_ops->erase(get_handle(), key, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    contains(const void* key, int* out_found) noexcept
    {
        ice::sonic::Status status;
        m_ops->contains(get_handle(), key, out_found, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void size(size_t* out_size) noexcept
    {
        m_ops->size(get_handle(), out_size);
    }

    void for_each(TF_MapVisitor visitor, void* capture) noexcept
    {
        m_ops->for_each(get_handle(), visitor, capture);
    }

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }
};

} // namespace ice::sonic
