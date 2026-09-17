// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/subscription.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/subscription.h"

export module cc_abi_builder_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TFPubSubSubscriptionOps
{
public:
    static TFPubSubSubscriptionOps* create(void* ctx) noexcept
    {
        return static_cast<TFPubSubSubscriptionOps*>(ctx);
    }

    template<typename HandleT>
    static TFPubSubSubscriptionOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFPubSubSubscriptionOps*>(handle->plugin_data);
    }

    virtual ~TFPubSubSubscriptionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> unsubscribe() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> ack() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> nack() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    seek(const ice::sonic::TF_StringOps& position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_lag(TFPubSubIntFn completion, void* user_data) noexcept = 0;

    static TFPubSubSubscriptionOps* get_generic_vtable()
    {
        static TFPubSubSubscriptionOps vtable = {
            .struct_size = TF_UBSUBSUBSCRIPTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFPubSubSubscriptionOps::create(plugin_context);
            },
            .unsubscribe =
                [](TFPubSubSubscription* subscription) noexcept
            {
                auto* self = TFPubSubSubscriptionOps::create(subscription);
                auto res = self->unsubscribe();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .ack =
                [](TFPubSubSubscription* subscription, TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubSubscriptionOps::create(subscription);
                auto res = self->ack();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .nack =
                [](TFPubSubSubscription* subscription, TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubSubscriptionOps::create(subscription);
                auto res = self->nack();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .seek =
                [](TFPubSubSubscription* subscription,
                   const TF_String* position,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubSubscriptionOps::create(subscription);
                auto res = self->seek(ice::sonic::TF_StringOps::wrap(position));
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .get_lag =
                [](TFPubSubSubscription* subscription,
                   TFPubSubIntFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto* self = TFPubSubSubscriptionOps::create(subscription);
                auto res = self->get_lag(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
