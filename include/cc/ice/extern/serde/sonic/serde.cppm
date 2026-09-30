// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/serde/serde.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/serde/serde.h"

export module cc_ice_extern_serde_sonic:serde;

import std;
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

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }

    void get_content_type(const ice::sonic::String& out_content_type) noexcept
    {
        m_ops->get_content_type(get_handle(), out_content_type.get_handle());
    }

    void get_format_name(const ice::sonic::String& out_format_name) noexcept
    {
        m_ops->get_format_name(get_handle(), out_format_name.get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    encode(const ice::sonic::String& value_json, const ice::sonic::String& out_encoded) noexcept
    {
        ice::sonic::Status status;
        m_ops->encode(
            get_handle(),
            value_json.get_handle(),
            out_encoded.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    decode(const ice::sonic::String& data, const ice::sonic::String& out_json) noexcept
    {
        ice::sonic::Status status;
        m_ops->decode(get_handle(), data.get_handle(), out_json.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
