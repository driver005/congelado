// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/channel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/channel.h"
#include "include/c/intern/map.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_pubsub_builder:channel;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class PubSubChannel
{
public:
    explicit PubSubChannel(
        const ::TF_MapOps* Map_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* Vector_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Map_ops = Map_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_Vector_ops = Vector_ops;
    }

    PubSubChannel(const PubSubChannel&) = delete;
    PubSubChannel& operator=(const PubSubChannel&) = delete;

    static PubSubChannel& from_handle(void* ctx) noexcept
    {
        return *static_cast<PubSubChannel*>(ctx);
    }

    template<typename HandleT>
    static PubSubChannel& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<PubSubChannel*>(handle->plugin_data);
    }

    virtual ~PubSubChannel() = default;
    virtual void destroy() noexcept = 0;
    virtual void create_channel(
        const ice::sonic::String& name,
        const ice::sonic::Map& config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_config(
        const ice::sonic::String& name,
        const ice::sonic::Map& out_config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_config(
        const ice::sonic::String& name,
        const ice::sonic::Map& config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stats(
        const ice::sonic::String& name,
        const ice::sonic::Map& out_stats,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    purge(const ice::sonic::String& name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void set_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& dead_letter_channel,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void list_dead_letters(
        const ice::sonic::String& target_channel,
        const ice::sonic::Vector& out_payloads,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void requeue_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& payload,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    list(const ice::sonic::Vector& out_channels, const ice::sonic::Status& out_status) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFPubSubChannel*)) noexcept
    {
        m_vtable = ::TFPubSubChannelOps{
            .struct_size = TF_OFFSET_OF_END(::TFPubSubChannelOps, list),

            .create = create,
            .destroy =
                [](::TFPubSubChannel* handle) noexcept
            {
                auto& self = PubSubChannel::from_handle(handle);
                self.destroy();
            },
            .create_channel =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* name,
                   const ::TF_Map* config,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.create_channel(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Map>{}, config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .drop =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* name,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.drop(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_config =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* name,
                   ::TF_Map* out_config,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.get_config(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Map>{}, out_config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_config =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* name,
                   const ::TF_Map* config,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.set_config(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Map>{}, config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stats =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* name,
                   ::TF_Map* out_stats,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.get_stats(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Map>{}, out_stats),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .purge =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* name,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.purge(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_dead_letter =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* target_channel,
                   const ::TF_String* dead_letter_channel,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.set_dead_letter(
                    self.wrap(std::type_identity<ice::sonic::String>{}, target_channel),
                    self.wrap(std::type_identity<ice::sonic::String>{}, dead_letter_channel),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_dead_letters =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* target_channel,
                   ::TF_Vector* out_payloads,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.list_dead_letters(
                    self.wrap(std::type_identity<ice::sonic::String>{}, target_channel),
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_payloads),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .requeue_dead_letter =
                [](::TFPubSubChannel* channel,
                   const ::TF_String* target_channel,
                   const ::TF_String* payload,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.requeue_dead_letter(
                    self.wrap(std::type_identity<ice::sonic::String>{}, target_channel),
                    self.wrap(std::type_identity<ice::sonic::String>{}, payload),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list =
                [](::TFPubSubChannel* channel,
                   ::TF_Vector* out_channels,
                   ::TF_Status* out_status) noexcept
            {
                auto& self = PubSubChannel::from_handle(channel);
                self.list(
                    self.wrap(std::type_identity<ice::sonic::Vector>{}, out_channels),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::Map wrap(std::type_identity<ice::sonic::Map>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::Map{m_Map_ops, const_cast<::TF_Map*>(handle)};
    }

    ice::sonic::Status
    wrap(std::type_identity<ice::sonic::Status>, const ::TF_Status* handle) const noexcept
    {
        return ice::sonic::Status{m_Status_ops, const_cast<::TF_Status*>(handle)};
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, const_cast<::TF_String*>(handle)};
    }

    ice::sonic::Vector
    wrap(std::type_identity<ice::sonic::Vector>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::Vector{m_Vector_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TFPubSubChannelOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFPubSubChannel& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFPubSubChannelOps*>(&m_vtable));
    }

private:
    ::TFPubSubChannelOps m_vtable;
    ::TFPubSubChannel m_handle;

    const ::TF_MapOps* m_Map_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_Vector_ops{nullptr};
};

} // namespace ice::builder
