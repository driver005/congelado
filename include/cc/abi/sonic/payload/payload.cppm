module;

#include "c/extern/payload/payload.h"

export module cc_abi_sonic_payload;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

// Runtime — the mainframe-facing payload handle. Same in-process/cross-plugin duality as
// ice::sonic::Cache and ice::sonic::Generator.
class Payload : public ice::sonic::Runtime<Payload, TF_Payload>
{
public:
    explicit Payload(TF_Payload* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "payload";

    [[nodiscard]] std::expected<void, ice::Status> write(
        ice::PayloadType type,
        const ice::String& data,
        TF_Payload_CompletionFn completion,
        void* cb_user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->write(
            get_handle(),
            ice::payload_type_to_c(type),
            data.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    read(const ice::String& reference, TF_Payload_CompletionFn completion, void* cb_user_data) noexcept
    {
        ice::Status status;
        m_ops->read(
            get_handle(),
            reference.get_handle(),
            completion,
            cb_user_data,
            status.get_handle()
        );
        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    ice::String get_name() const noexcept
    {
        ice::String out;
        m_ops->get_name(get_handle(), out.get_handle());
        return out;
    }

private:
};

} // namespace ice::sonic
