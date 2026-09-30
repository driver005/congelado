// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/publish.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/publish.h"

export module cc_ice_extern_pubsub_builder:publish;

import std;

export namespace ice::builder {

class TFPubSubPublishOps
{
public:
    TFPubSubPublishOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFPubSubPublishOps(const TFPubSubPublishOps&) = delete;
    TFPubSubPublishOps& operator=(const TFPubSubPublishOps&) = delete;

    static TFPubSubPublishOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFPubSubPublishOps*>(ctx);
    }

    template<typename HandleT>
    static TFPubSubPublishOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFPubSubPublishOps*>(handle->plugin_data);
    }

    virtual ~TFPubSubPublishOps() = default;
    virtual void destroy() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> publish(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload,
        int retain
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> publish_batch(
        const ice::sonic::String& channel,
        const ice::sonic::TF_VectorOps& payloads,
        TFPubSubAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status>
    flush(TFPubSubAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::sonic::Status> get_retained(
        const ice::sonic::String& channel,
        TFPubSubRetainedFn completion,
        void* user_data
    ) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFPubSubPublishOps{
            .struct_size = TF_UBSUBPUBLISH_STRUCT_SIZE,
            .destroy =
                [](TFPubSubPublish* publish) noexcept
            {
                TFPubSubPublishOps::from_handle(publish).destroy();
            },
            .publish =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   const TF_String* payload,
                   int retain,
                   TF_Status* out_status) noexcept
            {
                auto res = TFPubSubPublishOps::from_handle(publish).publish(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::String::wrap(payload),
                    retain
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .publish_batch =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   const TF_Vector* payloads,
                   TFPubSubAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TFPubSubPublishOps::from_handle(publish).publish_batch(
                    ice::sonic::String::wrap(channel),
                    ice::sonic::TF_VectorOps::wrap(payloads),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .flush =
                [](TFPubSubPublish* publish,
                   TFPubSubAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TFPubSubPublishOps::from_handle(publish).flush(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_retained =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   TFPubSubRetainedFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto res = TFPubSubPublishOps::from_handle(publish).get_retained(
                    ice::sonic::String::wrap(channel),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFPubSubPublishOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFPubSubPublish& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFPubSubPublishOps m_vtable;
    TFPubSubPublish m_handle;
};

} // namespace ice::builder
