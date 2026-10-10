// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/subscription.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/subscription.h"
#include "include/c/extern/registration/registration.h"

export module cc_ice_extern_pubsub_sonic:subscription;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFPubSubSubscriptionOps :
    public ice::sonic::Runtime<::TFPubSubSubscriptionOps, ::TFPubSubSubscription>
{
public:
    TFPubSubSubscriptionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, type.get_handle(), provider.get_handle())
    {
    }

    TFPubSubSubscriptionOps(
        const ::TF_RegistrationOps& registry_ops,
        ::TF_Registration* registry_handle,
        ::TFPubSubSubscription* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry_ops, registry_handle, handle, type.get_handle(), provider.get_handle())
    {
    }

    explicit TFPubSubSubscriptionOps(const ::TFPubSubSubscriptionOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFPubSubSubscriptionOps(
        const ::TFPubSubSubscriptionOps* ops,
        ::TFPubSubSubscription* handle
    ) noexcept :
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

    void unsubscribe() const noexcept
    {
        m_ops->unsubscribe(get_handle());
    }

    void ack(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->ack(get_handle(), out_status.get_handle());
    }

    void nack(const ice::sonic::Status& out_status) const noexcept
    {
        m_ops->nack(get_handle(), out_status.get_handle());
    }

    void seek(
        const ice::sonic::String& position,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->seek(get_handle(), position.get_handle(), out_status.get_handle());
    }

    void get_lag(
        TFPubSubIntFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_lag(get_handle(), completion, user_data, out_status.get_handle());
    }
};

} // namespace ice::sonic
