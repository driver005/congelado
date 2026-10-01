// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/publish.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/publish.h"

export module cc_ice_extern_pubsub_sonic:publish;

import std;
import cc_ice_intern_sonic;

export namespace ice::sonic {

class TFPubSubPublishOps : public ice::sonic::Runtime<::TFPubSubPublishOps, ::TFPubSubPublish>
{
public:
    template<typename Registry>
    TFPubSubPublishOps(
        Registry& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, type, provider)
    {
    }

    template<typename Registry>
    TFPubSubPublishOps(
        Registry& registry,
        ::TFPubSubPublish* handle,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) noexcept :
        Runtime(registry, handle, type, provider)
    {
    }

    explicit TFPubSubPublishOps(const ::TFPubSubPublishOps* ops) noexcept :
        Runtime(ops)
    {
    }

    TFPubSubPublishOps(const ::TFPubSubPublishOps* ops, ::TFPubSubPublish* handle) noexcept :
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

    void publish(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload,
        int retain,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->publish(
            get_handle(),
            channel.get_handle(),
            payload.get_handle(),
            retain,
            out_status.get_handle()
        );
    }

    void publish_batch(
        const ice::sonic::String& channel,
        const ice::sonic::TF_VectorOps& payloads,
        TFPubSubAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->publish_batch(
            get_handle(),
            channel.get_handle(),
            payloads.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }

    void flush(
        TFPubSubAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->flush(get_handle(), completion, user_data, out_status.get_handle());
    }

    void get_retained(
        const ice::sonic::String& channel,
        TFPubSubRetainedFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) const noexcept
    {
        m_ops->get_retained(
            get_handle(),
            channel.get_handle(),
            completion,
            user_data,
            out_status.get_handle()
        );
    }
};

} // namespace ice::sonic
