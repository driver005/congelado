// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/channel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/channel.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/map.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_pubsub_builder:channel;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFPubSubChannelOps
{
public:
    explicit TFPubSubChannelOps(
        const ::TF_MapOps* TF_MapOps_ops,
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_TF_MapOps_ops = TF_MapOps_ops;
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void create_channel(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void
    drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_config(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& out_config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void set_config(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_stats(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& out_stats,
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
        const ice::sonic::TF_VectorOps& out_payloads,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void requeue_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& payload,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void list(
        const ice::sonic::TF_VectorOps& out_channels,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFPubSubChannel*)) noexcept
    {
        m_vtable = ::TFPubSubChannelOps{
            .struct_size = TF_OFFSET_OF_END(::TFPubSubChannelOps, list),

            .create = create,
            .destroy =
                [](TFPubSubChannel* handle) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(handle);
                self.destroy();
            },
            .create_channel =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.create_channel(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .drop =
                [](TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.drop(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_config =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   TF_Map* out_config,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.get_config(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_config =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   const TF_Map* config,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.set_config(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, config),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_stats =
                [](TFPubSubChannel* channel,
                   const TF_String* name,
                   TF_Map* out_stats,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.get_stats(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::TF_MapOps>{}, out_stats),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .purge =
                [](TFPubSubChannel* channel, const TF_String* name, TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.purge(
                    self.wrap(std::type_identity<ice::sonic::String>{}, name),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .set_dead_letter =
                [](TFPubSubChannel* channel,
                   const TF_String* target_channel,
                   const TF_String* dead_letter_channel,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.set_dead_letter(
                    self.wrap(std::type_identity<ice::sonic::String>{}, target_channel),
                    self.wrap(std::type_identity<ice::sonic::String>{}, dead_letter_channel),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list_dead_letters =
                [](TFPubSubChannel* channel,
                   const TF_String* target_channel,
                   TF_Vector* out_payloads,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.list_dead_letters(
                    self.wrap(std::type_identity<ice::sonic::String>{}, target_channel),
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_payloads),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .requeue_dead_letter =
                [](TFPubSubChannel* channel,
                   const TF_String* target_channel,
                   const TF_String* payload,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.requeue_dead_letter(
                    self.wrap(std::type_identity<ice::sonic::String>{}, target_channel),
                    self.wrap(std::type_identity<ice::sonic::String>{}, payload),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .list =
                [](TFPubSubChannel* channel,
                   TF_Vector* out_channels,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubChannelOps::from_handle(channel);
                self.list(
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, out_channels),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
    }

    ice::sonic::TF_MapOps
    wrap(std::type_identity<ice::sonic::TF_MapOps>, const ::TF_Map* handle) const noexcept
    {
        return ice::sonic::TF_MapOps{m_TF_MapOps_ops, const_cast<::TF_Map*>(handle)};
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

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
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
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry_ops.register_op(
            registry_handle,
            type.get_handle(),
            provider.get_handle(),
            const_cast<::TFPubSubChannelOps*>(&m_vtable)
        );
    }

private:
    ::TFPubSubChannelOps m_vtable;
    ::TFPubSubChannel m_handle;

    const ::TF_MapOps* m_TF_MapOps_ops{nullptr};

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
