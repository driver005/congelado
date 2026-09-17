// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/subscription.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/subscription.h"

export module cc_abi_sonic_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFPubSubSubscriptionOps :
    public ice::sonic::Runtime<TFPubSubSubscriptionOps, TFPubSubSubscriptionOps>
{
public:
    explicit TFPubSubSubscriptionOps(TFPubSubSubscriptionOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "pubsub";

    [[nodiscard]] std::expected<void, ice::Status> unsubscribe() noexcept
    {
        ice::Status status;
        m_ops->unsubscribe(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> ack() noexcept
    {
        ice::Status status;
        m_ops->ack(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> nack() noexcept
    {
        ice::Status status;
        m_ops->nack(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    seek(const ice::sonic::TF_StringOps& position) noexcept
    {
        ice::Status status;
        m_ops->seek(get_handle(), position.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_lag(TFPubSubIntFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->get_lag(get_handle(), completion, user_data, status.get_handle());

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
