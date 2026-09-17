// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/serde/serde.h"

export module cc_abi_sonic_serde;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_SerdeOps : public ice::sonic::Runtime<TF_SerdeOps, TF_SerdeOps>
{
public:
    explicit TF_SerdeOps(TF_SerdeOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "serde";

    [[nodiscard]] std::expected<void, ice::Status>
    get_content_type(const ice::sonic::TF_StringOps& out_content_type) noexcept
    {
        ice::Status status;
        m_ops->get_content_type(get_handle(), out_content_type.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_format_name(const ice::sonic::TF_StringOps& out_format_name) noexcept
    {
        ice::Status status;
        m_ops->get_format_name(get_handle(), out_format_name.get_handle() status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> encode(
        const ice::sonic::TF_StringOps& value_json,
        const ice::sonic::TF_StringOps& out_encoded
    ) noexcept
    {
        ice::Status status;
        m_ops->encode(
            get_handle(),
            value_json.get_handle(),
            out_encoded.get_handle() status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    decode(const ice::sonic::TF_StringOps& data, const ice::sonic::TF_StringOps& out_json) noexcept
    {
        ice::Status status;
        m_ops->decode(get_handle(), data.get_handle(), out_json.get_handle() status.get_handle());

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
