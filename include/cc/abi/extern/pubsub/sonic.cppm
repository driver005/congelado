// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/pubsub/pubsub.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/pubsub/pubsub.h"

export module cc_abi_sonic_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class Pubsub : public ice::sonic::Runtime<Pubsub, TF_PubSub>
{
public:
    explicit Pubsub(TF_PubSub* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "pubsub";

    [[nodiscard]] std::expected<void, ice::Status> is_connected() noexcept
    {
        ice::Status status;
        m_ops->is_connected(get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> publish(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload,
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
        const ice::sonic::String& channel,
        const ice::sonic::Vector& payloads,
        TF_PubSub_AckFn completion,
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
    flush(TF_PubSub_AckFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->flush(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> subscribe(
        const ice::sonic::String& pattern,
        TF_PubSub_Handler handler,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->subscribe(
            get_handle(),
            pattern.get_handle(),
            handler,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> subscribe_group(
        const ice::sonic::String& pattern,
        const ice::sonic::String& group_id,
        TF_PubSub_Handler handler,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->subscribe_group(
            get_handle(),
            pattern.get_handle(),
            group_id.get_handle(),
            handler,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> subscribe_once(
        const ice::sonic::String& pattern,
        TF_PubSub_Handler handler,
        void* user_data
    ) noexcept
    {
        ice::Status status;
        m_ops->subscribe_once(
            get_handle(),
            pattern.get_handle(),
            handler,
            user_data,
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

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

    [[nodiscard]] std::expected<void, ice::Status> seek(const ice::sonic::String& position) noexcept
    {
        ice::Status status;
        m_ops->seek(get_handle(), position.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_subscription_lag(TF_PubSub_IntFn completion, void* user_data) noexcept
    {
        ice::Status status;
        m_ops->get_subscription_lag(get_handle(), completion, user_data, status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> get_retained(
        const ice::sonic::String& channel,
        TF_PubSub_RetainedFn completion,
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

    [[nodiscard]] std::expected<void, ice::Status>
    create_channel(const ice::sonic::String& name, const ice::sonic::Map& config) noexcept
    {
        ice::Status status;
        m_ops->create_channel(
            get_handle(),
            name.get_handle(),
            config.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    delete_channel(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->delete_channel(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_channel_config(const ice::sonic::String& name, const ice::sonic::Map& out_config) noexcept
    {
        ice::Status status;
        m_ops->get_channel_config(
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

    [[nodiscard]] std::expected<void, ice::Status>
    set_channel_config(const ice::sonic::String& name, const ice::sonic::Map& config) noexcept
    {
        ice::Status status;
        m_ops->set_channel_config(
            get_handle(),
            name.get_handle(),
            config.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    get_channel_stats(const ice::sonic::String& name, const ice::sonic::Map& out_stats) noexcept
    {
        ice::Status status;
        m_ops->get_channel_stats(
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

    [[nodiscard]] std::expected<void, ice::Status>
    purge_channel(const ice::sonic::String& name) noexcept
    {
        ice::Status status;
        m_ops->purge_channel(get_handle(), name.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> set_dead_letter_channel(
        const ice::sonic::String& channel,
        const ice::sonic::String& dead_letter_channel
    ) noexcept
    {
        ice::Status status;
        m_ops->set_dead_letter_channel(
            get_handle(),
            channel.get_handle(),
            dead_letter_channel.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> list_dead_letters(
        const ice::sonic::String& channel,
        const ice::sonic::Vector& out_payloads
    ) noexcept
    {
        ice::Status status;
        m_ops->list_dead_letters(
            get_handle(),
            channel.get_handle(),
            out_payloads.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status> requeue_dead_letter(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload
    ) noexcept
    {
        ice::Status status;
        m_ops->requeue_dead_letter(
            get_handle(),
            channel.get_handle(),
            payload.get_handle(),
            status.get_handle()
        );

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_channels(const ice::sonic::Vector& out_channels) noexcept
    {
        ice::Status status;
        m_ops->list_channels(get_handle(), out_channels.get_handle(), status.get_handle());

        if (!status.ok()) {
            return std::unexpected{status};
        }
        return {};
    }

    [[nodiscard]] std::expected<void, ice::Status>
    list_subscriptions(const ice::sonic::Vector& out_patterns) noexcept
    {
        ice::Status status;
        m_ops->list_subscriptions(get_handle(), out_patterns.get_handle(), status.get_handle());

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
