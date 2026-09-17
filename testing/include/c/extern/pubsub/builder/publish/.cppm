// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/publish.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/publish.h"

export module cc_abi_builder_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFPubSubPublishOps
{
public:
    static TFPubSubPublishOps* create(void* ctx) noexcept
    {
        return static_cast<TFPubSubPublishOps*>(ctx);
    }

    template<typename HandleT>
    static TFPubSubPublishOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFPubSubPublishOps*>(handle->plugin_data);
    }

    virtual ~TFPubSubPublishOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> publish(
        const ice::sonic::TF_StringOps& channel,
        const ice::sonic::TF_StringOps& payload,
        int retain
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> publish_batch(
        const ice::sonic::TF_StringOps& channel,
        const ice::sonic::TF_VectorOps& payloads,
        TFPubSubAckFn completion,
        void* user_data
    ) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    flush(TFPubSubAckFn completion, void* user_data) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> get_retained(
        const ice::sonic::TF_StringOps& channel,
        TFPubSubRetainedFn completion,
        void* user_data
    ) noexcept = 0;

    static TFPubSubPublishOps* get_generic_vtable()
    {
        static TFPubSubPublishOps vtable = {
            .struct_size = TF_UBSUBPUBLISH_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFPubSubPublishOps::create(plugin_context);
            },
            .publish =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   const TF_String* payload,
                   int retain,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubPublishOps::create(publish);
                auto res = self->publish(
                    ice::sonic::TF_StringOps::wrap(channel),
                    ice::sonic::TF_StringOps::wrap(payload),
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
                auto* self = TFPubSubPublishOps::create(publish);
                auto res = self->publish_batch(
                    ice::sonic::TF_StringOps::wrap(channel),
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
                auto* self = TFPubSubPublishOps::create(publish);
                auto res = self->flush(completion, user_data);
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
                auto* self = TFPubSubPublishOps::create(publish);
                auto res = self->get_retained(
                    ice::sonic::TF_StringOps::wrap(channel),
                    completion,
                    user_data
                );
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
