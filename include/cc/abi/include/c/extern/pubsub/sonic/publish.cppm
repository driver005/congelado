// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/publish.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/publish.h"

export module cc_abi_sonic_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
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

    [[nodiscard]] std::expected<void, ice::Status> publish(
        const ice::sonic::TF_StringOps& channel,
        const ice::sonic::TF_StringOps& payload,
        int retain
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status> publish_batch(
        const ice::sonic::TF_StringOps& channel,
        const ice::sonic::TF_VectorOps& payloads,
        TFPubSubAckFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    [[nodiscard]] std::expected<void, ice::Status>
    flush(TFPubSubAckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->flush(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_retained(
        const ice::sonic::TF_StringOps& channel,
        TFPubSubRetainedFn completion,
        void* user_data
    ) noexcept
    {
        ice::Status status;
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

    ice::String get_name() const noexcept
    {
        ice::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::sonic
