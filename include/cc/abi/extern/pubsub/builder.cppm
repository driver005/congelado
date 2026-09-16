// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from c/extern/pubsub/pubsub.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "c/extern/pubsub/pubsub.h"
#include "c/intern/tf_status.h"
#include "c/intern/tf_tstring.h"

export module cc_abi_builder_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class Pubsub
{
public:
    static Pubsub* create(void* ctx) noexcept
    {
        return static_cast<Pubsub*>(ctx);
    }

    template<typename HandleT>
    static Pubsub* create(HandleT* handle) noexcept
    {
        return reinterpret_cast<Pubsub*>(handle);
    }

    virtual ~Pubsub() = default;
    [[nodiscard]] std::expected<void, ice::Status> is_connected() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> publish(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload,
        int retain
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> publish_batch(
        const ice::sonic::String& channel,
        const ice::sonic::Vector& payloads,
        TF_PubSub_AckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    flush(TF_PubSub_AckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> subscribe(
        const ice::sonic::String& pattern,
        TF_PubSub_Handler handler,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> subscribe_group(
        const ice::sonic::String& pattern,
        const ice::sonic::String& group_id,
        TF_PubSub_Handler handler,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> subscribe_once(
        const ice::sonic::String& pattern,
        TF_PubSub_Handler handler,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> unsubscribe() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> ack() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> nack() noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    seek(const ice::sonic::String& position) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_subscription_lag(TF_PubSub_IntFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> get_retained(
        const ice::sonic::String& channel,
        TF_PubSub_RetainedFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    create_channel(const ice::sonic::String& name, const ice::sonic::Map& config) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    delete_channel(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_channel_config(const ice::sonic::String& name, const ice::sonic::Map& out_config) noexcept =
        0;
    [[nodiscard]] std::expected<void, ice::Status>
    set_channel_config(const ice::sonic::String& name, const ice::sonic::Map& config) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    get_channel_stats(const ice::sonic::String& name, const ice::sonic::Map& out_stats) noexcept =
        0;
    [[nodiscard]] std::expected<void, ice::Status>
    purge_channel(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> set_dead_letter_channel(
        const ice::sonic::String& channel,
        const ice::sonic::String& dead_letter_channel
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> list_dead_letters(
        const ice::sonic::String& channel,
        const ice::sonic::Vector& out_payloads
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status> requeue_dead_letter(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload
    ) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_channels(const ice::sonic::Vector& out_channels) noexcept = 0;
    [[nodiscard]] std::expected<void, ice::Status>
    list_subscriptions(const ice::sonic::Vector& out_patterns) noexcept = 0;
    virtual ice::String get_name() const noexcept = 0;

    static TF_PubSub* get_generic_vtable()
    {
        static TF_PubSub vtable = {
            .struct_size = TF_PUBSUB_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete Pubsub::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },
            .is_connected =
                [](void* plugin_context) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->is_connected();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .publish =
                [](void* plugin_context,
                   const TF_String_Handle* channel,
                   const TF_String_Handle* payload,
                   int retain,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->publish(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::String::wrap(payload),
                    retain
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .publish_batch =
                [](void* plugin_context,
                   const TF_String_Handle* channel,
                   const TF_Vector_Handle* payloads,
                   TF_PubSub_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->publish_batch(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::Vector::wrap(payloads),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .flush =
                [](void* plugin_context,
                   TF_PubSub_AckFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->flush(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .subscribe =
                [](void* plugin_context,
                   const TF_String_Handle* pattern,
                   TF_PubSub_Handler handler,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->subscribe(ice::sonic::String::wrap(pattern), handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .subscribe_group =
                [](void* plugin_context,
                   const TF_String_Handle* pattern,
                   const TF_String_Handle* group_id,
                   TF_PubSub_Handler handler,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->subscribe_group(
                    ice::sonic::String::wrap(pattern),
                    ice::sonic::String::wrap(group_id),
                    handler,
                    user_data
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .subscribe_once =
                [](void* plugin_context,
                   const TF_String_Handle* pattern,
                   TF_PubSub_Handler handler,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res =
                    self->subscribe_once(ice::sonic::String::wrap(pattern), handler, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .unsubscribe =
                [](TF_PubSub_Subscription* subscription) noexcept
            {
                auto* self = Pubsub::create(subscription);
                auto res = self->unsubscribe();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .ack =
                [](TF_PubSub_Subscription* subscription, TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(subscription);
                auto res = self->ack();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .nack =
                [](TF_PubSub_Subscription* subscription, TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(subscription);
                auto res = self->nack();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .seek =
                [](TF_PubSub_Subscription* subscription,
                   const TF_String_Handle* position,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(subscription);
                auto res = self->seek(ice::sonic::String::wrap(position));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_subscription_lag =
                [](TF_PubSub_Subscription* subscription,
                   TF_PubSub_IntFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(subscription);
                auto res = self->get_subscription_lag(completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_retained =
                [](void* plugin_context,
                   const TF_String_Handle* channel,
                   TF_PubSub_RetainedFn completion,
                   void* user_data,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res =
                    self->get_retained(ice::sonic::String::wrap(channel), completion, user_data);
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .create_channel =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   const TF_Map_Handle* config,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->create_channel(
                    ice::sonic::String::wrap(name),
                    ice::sonic::Map::wrap(config)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .delete_channel =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->delete_channel(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_channel_config =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Map_Handle* out_config,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->get_channel_config(
                    ice::sonic::String::wrap(name),
                    ice::sonic::Map::wrap(out_config)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_channel_config =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   const TF_Map_Handle* config,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->set_channel_config(
                    ice::sonic::String::wrap(name),
                    ice::sonic::Map::wrap(config)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .get_channel_stats =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Map_Handle* out_stats,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->get_channel_stats(
                    ice::sonic::String::wrap(name),
                    ice::sonic::Map::wrap(out_stats)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .purge_channel =
                [](void* plugin_context,
                   const TF_String_Handle* name,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->purge_channel(ice::sonic::String::wrap(name));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .set_dead_letter_channel =
                [](void* plugin_context,
                   const TF_String_Handle* channel,
                   const TF_String_Handle* dead_letter_channel,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->set_dead_letter_channel(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::String::wrap(dead_letter_channel)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_dead_letters =
                [](void* plugin_context,
                   const TF_String_Handle* channel,
                   TF_Vector_Handle* out_payloads,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->list_dead_letters(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::Vector::wrap(out_payloads)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .requeue_dead_letter =
                [](void* plugin_context,
                   const TF_String_Handle* channel,
                   const TF_String_Handle* payload,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->requeue_dead_letter(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::String::wrap(payload)
                );
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_channels =
                [](void* plugin_context,
                   TF_Vector_Handle* out_channels,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->list_channels(ice::sonic::Vector::wrap(out_channels));
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .list_subscriptions =
                [](void* plugin_context,
                   TF_Vector_Handle* out_patterns,
                   TF_Status_Handle* status) noexcept
            {
                auto* self = Pubsub::create(plugin_context);
                auto res = self->list_subscriptions(ice::sonic::Vector::wrap(out_patterns));
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
