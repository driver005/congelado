module;

#include "c/extern/serde/serde.h"

export module cc_abi_sonic_serde;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing serde handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Serde : public ice::sonic::Runtime<Serde, TF_Serde>
{
public:
    explicit Serde(TF_Serde* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "serde";

    [[nodiscard]] std::expected<ice::String, ice::Status> encode(const ice::String& value_json)
    {
        ice::Status status;
        ice::String result;
        m_ops->encode(get_handle(), value_json.get_handle(), result.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    [[nodiscard]] std::expected<ice::String, ice::Status> decode(const ice::String& data)
    {
        ice::Status status;
        ice::String result;
        m_ops->decode(get_handle(), data.get_handle(), result.get_handle(), status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return result;
    }

    ice::String get_content_type() const noexcept
    {
        ice::String tf_content_type;
        m_ops->get_content_type(get_handle(), tf_content_type.get_handle());
        return tf_content_type;
    }

    ice::String get_format_name() const noexcept
    {
        ice::String tf_format_name;
        m_ops->get_format_name(get_handle(), tf_format_name.get_handle());
        return tf_format_name;
    }
};

} // namespace ice::sonic
