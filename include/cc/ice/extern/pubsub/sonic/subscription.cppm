// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/subscription.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/subscription.h"

export module cc_ice_extern_pubsub_sonic:subscription;

import std;
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

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void unsubscribe() noexcept
    {
        m_ops->unsubscribe(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> ack() noexcept
    {
        ice::sonic::Status status;
        m_ops->ack(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> nack() noexcept
    {
        ice::sonic::Status status;
        m_ops->nack(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    seek(const ice::sonic::String& position) noexcept
    {
        ice::sonic::Status status;
        m_ops->seek(get_handle(), position.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_lag(TFPubSubIntFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_lag(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
