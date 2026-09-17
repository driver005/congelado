// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/hive/hive.h"

export module cc_abi_sonic_hive;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_HiveOps : public ice::sonic::Runtime<TF_HiveOps, TF_HiveOps>
{
public:
    explicit TF_HiveOps(TF_HiveOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "hive";

    [[nodiscard]] std::expected<void, ice::Status> set_element_size(size_t element_size) noexcept
    {
        ice::Status status;
        m_ops->set_element_size(get_handle(), element_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    insert(const void* value, TFHiveSlot* out_slot) noexcept
    {
        ice::Status status;
        m_ops->insert(get_handle(), value, out_slot, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> erase(TFHiveSlot* slot) noexcept
    {
        ice::Status status;
        m_ops->erase(get_handle(), slot, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get(const TFHiveSlot* slot, const void** out_value) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), slot, out_value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    for_each(TF_HiveVisitor visitor, void* capture) noexcept
    {
        ice::Status status;
        m_ops->for_each(get_handle(), visitor, capture, status.get_handle());

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

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
