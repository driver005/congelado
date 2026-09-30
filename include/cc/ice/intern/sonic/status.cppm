// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/intern/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/intern/status.h"

export module cc_ice_intern_sonic:status;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Status : public ice::sonic::Runtime<Status, TF_StatusOps>
{
public:
    explicit Status(TF_StatusOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "intern";

    [[nodiscard]] std::expected<void, ice::sonic::Status> delete_status() noexcept
    {
        ice::sonic::Status status;
        m_ops->delete_status(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    void set_status(TF_Code code, const ice::sonic::String& msg) noexcept
    {
        m_ops->set_status(get_handle(), code, msg.get_handle());
    }

    void set_payload(const ice::sonic::String& key, const ice::sonic::String& value) noexcept
    {
        m_ops->set_payload(get_handle(), key.get_handle(), value.get_handle());
    }

    void for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept
    {
        m_ops->for_each_payload(get_handle(), visitor, capture);
    }

    void set_status_from_io_error(int error_code, const ice::sonic::String& context) noexcept
    {
        m_ops->set_status_from_io_error(get_handle(), error_code, context.get_handle());
    }

    void get_code(TF_Code* out_code) noexcept
    {
        m_ops->get_code(get_handle(), out_code);
    }

    void message(const ice::sonic::String& out_message) noexcept
    {
        m_ops->message(get_handle(), out_message.get_handle());
    }
};

} // namespace ice::sonic
