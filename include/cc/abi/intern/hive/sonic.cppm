// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/hive/hive.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/hive/hive.h"

export module cc_abi_sonic_hive;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Hive : public ice::sonic::Runtime<Hive, TF_Hive>
{
public:
    explicit Hive(TF_Hive* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "hive";

    [[nodiscard]] std::expected<void, ice::Status> new_hive(size_t element_size) noexcept
    {
        ice::Status status;
        m_ops->new_hive(get_handle(), element_size, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> insert(const void* value) noexcept
    {
        ice::Status status;
        m_ops->insert(get_handle(), value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> erase(TF_Hive_Slot* slot) noexcept
    {
        ice::Status status;
        m_ops->erase(get_handle(), slot, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get(const TF_Hive_Slot* slot) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), slot, status.get_handle());

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

    [[nodiscard]] std::expected<void, ice::Status> size() noexcept
    {
        ice::Status status;
        m_ops->size(get_handle(), status.get_handle());

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
