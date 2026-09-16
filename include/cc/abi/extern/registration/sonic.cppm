// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/registration/registration.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/registration/registration.h"

export module cc_abi_sonic_registration;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Registration : public ice::sonic::Runtime<Registration, TF_RegistrationOps>
{
public:
    explicit Registration(TF_RegistrationOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "registration";

    [[nodiscard]] std::expected<void, ice::Status> new_registration() noexcept
    {
        ice::Status status;
        m_ops->new_registration(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> register_op(
        const ice::sonic::String& type,
        const ice::sonic::String& name,
        void* value
    ) noexcept
    {
        ice::Status status;
        m_ops->register_op(
            get_handle(),
            type.get_handle(),
            name.get_handle(),
            value,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get(const ice::sonic::String& type, const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->get(get_handle(), type.get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    unregister(const ice::sonic::String& type, const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->unregister(get_handle(), type.get_handle(), name.get_handle(), status.get_handle());

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
