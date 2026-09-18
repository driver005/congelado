// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/channel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/channel.h"

export module cc_ice_builder_pubsub:channel;

import std;

export namespace ice::builder {

class TFPubSubChannelOps
{
public:
    static TFPubSubChannelOps* create(void* ctx) noexcept
    {
        return static_cast<TFPubSubChannelOps*>(ctx);
    }

    template<typename HandleT>
    static TFPubSubChannelOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFPubSubChannelOps*>(handle->plugin_data);
    }

    virtual ~TFPubSubChannelOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create(const ice::sonic::TF_StringOps& name, const ice::sonic::TF_MapOps& config) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    drop(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_config(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_MapOps& out_config
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_config(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_MapOps& config
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_stats(
        const ice::sonic::TF_StringOps& name,
        const ice::sonic::TF_MapOps& out_stats
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    purge(const ice::sonic::TF_StringOps& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_dead_letter(
        const ice::sonic::TF_StringOps& target_channel,
        const ice::sonic::TF_StringOps& dead_letter_channel
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> list_dead_letters(
        const ice::sonic::TF_StringOps& target_channel,
        const ice::sonic::TF_VectorOps& out_payloads
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> requeue_dead_letter(
        const ice::sonic::TF_StringOps& target_channel,
        const ice::sonic::TF_StringOps& payload
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list(const ice::sonic::TF_VectorOps& out_channels) noexcept = 0;

    static TFPubSubChannelOps* get_generic_vtable()
    {
        static TFPubSubChannelOps vtable = {
            .struct_size = TF_UBSUBCHANNEL_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFPubSubChannelOps::create(plugin_context);
            },
            .create =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->create(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_MapOps::wrap(config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .drop =
                [](TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->drop(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_config =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   TF_Map* out_config,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->get_config(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_MapOps::wrap(out_config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_config =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->set_config(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_MapOps::wrap(config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_stats =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   TF_Map* out_stats,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->get_stats(
                    ice::sonic::TF_StringOps::wrap(name),
                    ice::sonic::TF_MapOps::wrap(out_stats)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .purge =
                [](TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->purge(ice::sonic::TF_StringOps::wrap(name));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .set_dead_letter =
                [](TFPubSubChannel* channel,
                   const TF_String* target_channel,
                   const TF_String* dead_letter_channel,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->set_dead_letter(
                    ice::sonic::TF_StringOps::wrap(target_channel),
                    ice::sonic::TF_StringOps::wrap(dead_letter_channel)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list_dead_letters =
                [](TFPubSubChannel* channel,
                   const TF_String* target_channel,
                   TF_Vector* out_payloads,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->list_dead_letters(
                    ice::sonic::TF_StringOps::wrap(target_channel),
                    ice::sonic::TF_VectorOps::wrap(out_payloads)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .requeue_dead_letter =
                [](TFPubSubChannel* channel,
                   const TF_String* target_channel,
                   const TF_String* payload,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->requeue_dead_letter(
                    ice::sonic::TF_StringOps::wrap(target_channel),
                    ice::sonic::TF_StringOps::wrap(payload)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .list =
                [](TFPubSubChannel* channel,
                   TF_Vector* out_channels,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubChannelOps::create(channel);
                auto res = self->list(ice::sonic::TF_VectorOps::wrap(out_channels));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
