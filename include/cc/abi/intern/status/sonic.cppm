// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/status/status.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/status/status.h"

export module cc_abi_sonic_status;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Status : public ice::sonic::Runtime<Status, TF_Status>
{
public:
    explicit Status(TF_Status* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "status";

    [[nodiscard]] std::expected<void, ice::Status> new_status() noexcept
    {
        ice::Status status;
        m_ops->new_status(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> delete_status() noexcept
    {
        ice::Status status;
        m_ops->delete_status(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_status(TF_Code code, const char* msg) noexcept
    {
        ice::Status status;
        m_ops->set_status(get_handle(), code, msg, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_payload(const char* key, const char* value) noexcept
    {
        ice::Status status;
        m_ops->set_payload(get_handle(), key, value, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    for_each_payload(TF_PayloadVisitor visitor, void* capture) noexcept
    {
        ice::Status status;
        m_ops->for_each_payload(get_handle(), visitor, capture, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    set_status_from_io_error(int error_code, const char* context) noexcept
    {
        ice::Status status;
        m_ops->set_status_from_io_error(get_handle(), error_code, context, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_code() noexcept
    {
        ice::Status status;
        m_ops->get_code(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> message() noexcept
    {
        ice::Status status;
        m_ops->message(get_handle(), status.get_handle());

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
