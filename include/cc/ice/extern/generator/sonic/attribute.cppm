// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/attribute.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/attribute.h"

export module cc_ice_extern_generator_sonic:attribute;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFGeneratorAttributeOps :
    public ice::sonic::Runtime<TFGeneratorAttributeOps, TFGeneratorAttributeOps>
{
public:
    explicit TFGeneratorAttributeOps(TFGeneratorAttributeOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status> set_name(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->set_name(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_description(const ice::sonic::String& description) noexcept
    {
        ice::Status status;
        m_ops->set_description(get_handle(), description.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_full_type(const ice::sonic::String& full_type) noexcept
    {
        ice::Status status;
        m_ops->set_full_type(get_handle(), full_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_base_type(const ice::sonic::String& base_type) noexcept
    {
        ice::Status status;
        m_ops->set_base_type(get_handle(), base_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_is_list(_Bool is_list) noexcept
    {
        ice::Status status;
        m_ops->set_is_list(get_handle(), is_list, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_description(const ice::sonic::String& out_description) noexcept
    {
        ice::Status status;
        m_ops->get_description(get_handle(), out_description.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_full_type(const ice::sonic::String& out_full_type) noexcept
    {
        ice::Status status;
        m_ops->get_full_type(get_handle(), out_full_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_base_type(const ice::sonic::String& out_base_type) noexcept
    {
        ice::Status status;
        m_ops->get_base_type(get_handle(), out_base_type.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_list(int* out_is_list) noexcept
    {
        ice::Status status;
        m_ops->is_list(get_handle(), out_is_list, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    sonic::String get_name() const noexcept
    {
        sonic::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
