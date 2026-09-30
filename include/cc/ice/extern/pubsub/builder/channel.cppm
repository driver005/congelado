// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/channel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/channel.h"

export module cc_ice_extern_pubsub_builder:channel;

import std;

export namespace ice::builder {

class TFPubSubChannelOps
{
public:
    TFPubSubChannelOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFPubSubChannelOps(const TFPubSubChannelOps&) = delete;
    TFPubSubChannelOps& operator=(const TFPubSubChannelOps&) = delete;

    static TFPubSubChannelOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFPubSubChannelOps*>(ctx);
    }

    template<typename HandleT>
    static TFPubSubChannelOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFPubSubChannelOps*>(handle->plugin_data);
    }

    virtual ~TFPubSubChannelOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    create(const ice::sonic::String& name, const ice::sonic::TF_MapOps& config) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    drop(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_config(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& out_config
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    set_config(const ice::sonic::String& name, const ice::sonic::TF_MapOps& config) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_stats(const ice::sonic::String& name, const ice::sonic::TF_MapOps& out_stats) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    purge(const ice::sonic::String& name) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> set_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& dead_letter_channel
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> list_dead_letters(
        const ice::sonic::String& target_channel,
        const ice::sonic::TF_VectorOps& out_payloads
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> requeue_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& payload
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    list(const ice::sonic::TF_VectorOps& out_channels) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFPubSubChannelOps{
            .struct_size = TF_UBSUBCHANNEL_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFPubSubChannelOps>{
                    &TFPubSubChannelOps::from_handle(plugin_context)
                };
            },
            .create =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto res = TFPubSubChannelOps::from_handle(channel).create(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TF_MapOps::wrap(config)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .drop =
                [](TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res =
                    TFPubSubChannelOps::from_handle(channel).drop(ice::sonic::String::wrap(name));
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
                auto res = TFPubSubChannelOps::from_handle(channel).get_config(
                    ice::sonic::String::wrap(name),
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
                auto res = TFPubSubChannelOps::from_handle(channel).set_config(
                    ice::sonic::String::wrap(name),
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
                auto res = TFPubSubChannelOps::from_handle(channel).get_stats(
                    ice::sonic::String::wrap(name),
                    ice::sonic::TF_MapOps::wrap(out_stats)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .purge =
                [](TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto res =
                    TFPubSubChannelOps::from_handle(channel).purge(ice::sonic::String::wrap(name));
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
                auto res = TFPubSubChannelOps::from_handle(channel).set_dead_letter(
                    ice::sonic::String::wrap(target_channel),
                    ice::sonic::String::wrap(dead_letter_channel)
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
                auto res = TFPubSubChannelOps::from_handle(channel).list_dead_letters(
                    ice::sonic::String::wrap(target_channel),
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
                auto res = TFPubSubChannelOps::from_handle(channel).requeue_dead_letter(
                    ice::sonic::String::wrap(target_channel),
                    ice::sonic::String::wrap(payload)
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
                auto res = TFPubSubChannelOps::from_handle(channel).list(
                    ice::sonic::TF_VectorOps::wrap(out_channels)
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFPubSubChannelOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFPubSubChannel& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFPubSubChannelOps m_vtable;
    TFPubSubChannel m_handle;
};

} // namespace ice::builder
