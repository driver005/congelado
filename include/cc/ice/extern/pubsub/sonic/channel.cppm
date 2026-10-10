// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/channel.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/channel.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_pubsub_sonic:channel;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFPubSubChannelOps : public ice::sonic::Runtime<::TFPubSubChannelOps, ::TFPubSubChannel>
{
public:
    TFPubSubChannelOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFPubSubChannelOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFPubSubChannel* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFPubSubChannelOps(const ::TFPubSubChannelOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFPubSubChannelOps(const ::TFPubSubChannelOps* ops, ::TFPubSubChannel* handle) noexcept :
        Runtime(ops, handle)
    {
    }

    void create() const noexcept
    {
        m_ops->create(get_handle());
    }

    void destroy() const noexcept
    {
        m_ops->destroy(get_handle());
    }

    void create_channel(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->create_channel(
            get_handle(),
            name.get_handle(),
            config.get_handle(),
            out_status.get_handle()
        );
    }

    void drop(const ice::sonic::String& name, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->drop(get_handle(), name.get_handle(), out_status.get_handle());
    }

    void get_config(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& out_config,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_config(
            get_handle(),
            name.get_handle(),
            out_config.get_handle(),
            out_status.get_handle()
        );
    }

    void set_config(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& config,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_config(
            get_handle(),
            name.get_handle(),
            config.get_handle(),
            out_status.get_handle()
        );
    }

    void get_stats(
        const ice::sonic::String& name,
        const ice::sonic::TF_MapOps& out_stats,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_stats(
            get_handle(),
            name.get_handle(),
            out_stats.get_handle(),
            out_status.get_handle()
        );
    }

    void purge(const ice::sonic::String& name, const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->purge(get_handle(), name.get_handle(), out_status.get_handle());
    }

    void set_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& dead_letter_channel,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->set_dead_letter(
            get_handle(),
            target_channel.get_handle(),
            dead_letter_channel.get_handle(),
            out_status.get_handle()
        );
    }

    void list_dead_letters(
        const ice::sonic::String& target_channel,
        const ice::sonic::TF_VectorOps& out_payloads,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list_dead_letters(
            get_handle(),
            target_channel.get_handle(),
            out_payloads.get_handle(),
            out_status.get_handle()
        );
    }

    void requeue_dead_letter(
        const ice::sonic::String& target_channel,
        const ice::sonic::String& payload,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->requeue_dead_letter(
            get_handle(),
            target_channel.get_handle(),
            payload.get_handle(),
            out_status.get_handle()
        );
    }

    void list(
        const ice::sonic::TF_VectorOps& out_channels,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->list(get_handle(), out_channels.get_handle(), out_status.get_handle());
    }
};

} // namespace ice::sonic
