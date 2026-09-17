// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/generator/typeinfo.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/generator/typeinfo.h"

export module cc_abi_sonic_generator;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_TypeInfoOps : public ice::sonic::Runtime<TF_TypeInfoOps, TF_TypeInfoOps>
{
public:
    explicit TF_TypeInfoOps(TF_TypeInfoOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "generator";

    [[nodiscard]] std::expected<void, ice::Status>
    set_type_attr_name(const ice::sonic::TF_StringOps& type_attr_name) noexcept
    {
        ice::Status status;
        m_ops->set_type_attr_name(get_handle(), type_attr_name.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_data_type(int data_type) noexcept
    {
        ice::Status status;
        m_ops->set_data_type(get_handle(), data_type status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_read_only(_Bool read_only) noexcept
    {
        ice::Status status;
        m_ops->set_read_only(get_handle(), read_only status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_list(_Bool is_list) noexcept
    {
        ice::Status status;
        m_ops->set_list(get_handle(), is_list status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_type_attr_name(const ice::sonic::TF_StringOps& out_type_attr_name) noexcept
    {
        ice::Status status;
        m_ops->get_type_attr_name(
            get_handle(),
            out_type_attr_name.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_data_type(int* out_data_type) noexcept
    {
        ice::Status status;
        m_ops->get_data_type(get_handle(), out_data_type status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_read_only(int* out_is_read_only) noexcept
    {
        ice::Status status;
        m_ops->is_read_only(get_handle(), out_is_read_only status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> is_list(int* out_is_list) noexcept
    {
        ice::Status status;
        m_ops->is_list(get_handle(), out_is_list status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    virtual ice::String get_name() const noexcept = 0;

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
