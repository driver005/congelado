// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/channel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/channel.h"

export module cc_ice_extern_pubsub_sonic:channel;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFPubSubChannelOps : public ice::sonic::Runtime<TFPubSubChannelOps, TFPubSubChannelOps>
{
public:
    explicit TFPubSubChannelOps(TFPubSubChannelOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "pubsub";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    create(const ice::sonic::String& name, const ice::sonic::TF_MapOps& config) noexcept
    {
        ice::sonic::Status status;
        m_ops->create(get_handle(), name.get_handle(), config.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    drop(const ice::sonic::String& name) noexcept
    {
        ice::sonic::Status status;
        m_ops->drop(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_config(const ice::sonic::String& name, const ice::sonic::TF_MapOps& out_config) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_config(
            get_handle(),
            name.get_handle(),
            out_config.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    set_config(const ice::sonic::String& name, const ice::sonic::TF_MapOps& config) noexcept
    {
        ice::sonic::Status status;
        m_ops
            ->set_config(get_handle(), name.get_handle(), config.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    get_stats(const ice::sonic::String& name, const ice::sonic::TF_MapOps& out_stats) noexcept
    {
        ice::sonic::Status status;
        m_ops->get_stats(
            get_handle(),
            name.get_handle(),
            out_stats.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    purge(const ice::sonic::String& name) noexcept
    {
        ice::sonic::Status status;
        m_ops->purge(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> set_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& dead_letter_channel
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->set_dead_letter(
            get_handle(),
            target_channel.get_handle(),
            dead_letter_channel.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> list_dead_letters(
        const ice::sonic::String& target_channel,
        const ice::sonic::TF_VectorOps& out_payloads
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->list_dead_letters(
            get_handle(),
            target_channel.get_handle(),
            out_payloads.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status> requeue_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& payload
    ) noexcept
    {
        ice::sonic::Status status;
        m_ops->requeue_dead_letter(
            get_handle(),
            target_channel.get_handle(),
            payload.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::sonic::Status>
    list(const ice::sonic::TF_VectorOps& out_channels) noexcept
    {
        ice::sonic::Status status;
        m_ops->list(get_handle(), out_channels.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }
};

} // namespace ice::sonic
