module;

#include "c/extern/serde/serde.h"

export module cc_abi_sonic_serde;

import std;
import cc_abi_sonic_intern;
import cc_abi_primitives;
import cc_abi_sonic_registration;
export namespace ice::sonic {

// Runtime — the mainframe-facing serde handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Serde : public ice::sonic::Runtime<Serde, TF_Serde, /*PassNameToFactory=*/true>
{
public:
    static constexpr std::string_view domain_name = "serde";

    std::expected<ice::String, ice::Status>
    encode(const ice::String& value_json)
    {


        ice::Status status;
        TF_TString* out_encoded = nullptr;
        this->m_ops->encode(this->get_handle(), value_json.get_handle(), &out_encoded, status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        ice::String result{out_encoded};
        TF_Serde_FreeString(out_encoded);
        return result;
    }

    std::expected<ice::String, ice::Status>
    decode(const ice::String& data)
    {


        ice::Status status;
        TF_TString* out_json = nullptr;
        this->m_ops->decode(this->get_handle(), data.get_handle(), &out_json, status.get_handle());
        if (!status.ok()) {
            return std::unexpected{status};
        }
        ice::String result{out_json};
        TF_Serde_FreeString(out_json);
        return result;
    }

    ice::String get_content_type() const
    {


        ice::String tf_content_type;
        this->m_ops->get_content_type(this->get_handle(), tf_content_type.get_handle());
        return std::move(tf_content_type);
    }

    ice::String get_format_name() const
    {


        ice::String tf_format_name;
        this->m_ops->get_format_name(this->get_handle(), tf_format_name.get_handle());
        return std::move(tf_format_name);
    }

public:
    explicit Serde(TF_Serde* ops, void* plugin_context) : Runtime(ops, plugin_context) {}
};

} // namespace ice::sonic
