// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/publish.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/publish.h"

export module cc_ice_extern_pubsub_sonic:publish;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFPubSubPublishOps : public ice::sonic::Runtime<TFPubSubPublishOps, TFPubSubPublishOps>
{
public:
    explicit TFPubSubPublishOps(TFPubSubPublishOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "pubsub";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> publish(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload,
        int retain
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->publish(
            get_handle(),
            channel.get_handle(),
            payload.get_handle(),
            retain,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> publish_batch(
        const ice::sonic::String& channel,
        const ice::sonic::TF_VectorOps& payloads,
        TFPubSubAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->publish_batch(
            get_handle(),
            channel.get_handle(),
            payloads.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    flush(TFPubSubAckFn completion, void* user_data) noexcept
    {
        ice::sonic::Status status;
        m_ops->flush(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> get_retained(
        const ice::sonic::String& channel,
        TFPubSubRetainedFn completion,
        void* user_data
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_retained(
            get_handle(),
            channel.get_handle(),
            completion,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
