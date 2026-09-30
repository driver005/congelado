// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/subscription.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/subscription.h"

export module cc_ice_extern_pubsub_builder:subscription;

import std;

export namespace ice::builder {

class TFPubSubSubscriptionOps
{
public:
    TFPubSubSubscriptionOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFPubSubSubscriptionOps(const TFPubSubSubscriptionOps&) = delete;
    TFPubSubSubscriptionOps& operator=(const TFPubSubSubscriptionOps&) = delete;

    static TFPubSubSubscriptionOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFPubSubSubscriptionOps*>(ctx);
    }

    template<typename HandleT>
    static TFPubSubSubscriptionOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFPubSubSubscriptionOps*>(handle->plugin_data);
    }

    virtual ~TFPubSubSubscriptionOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> unsubscribe() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> ack() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status> nack() noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    seek(const ice::sonic::String& position) noexcept = 0;
    [[nodiscard]] virtual std::expected<void, ice::Status>
    get_lag(TFPubSubIntFn completion, void* user_data) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFPubSubSubscriptionOps{
            .struct_size = TF_UBSUBSUBSCRIPTION_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFPubSubSubscriptionOps>{
                    &TFPubSubSubscriptionOps::from_handle(plugin_context)
                };
            },
            .unsubscribe =
                [](TFPubSubSubscription* subscription) noexcept
            {
                auto res = TFPubSubSubscriptionOps::from_handle(subscription).unsubscribe();
                if (!res) {
                    res.error().to_c(status);
                }
            },
            .ack =
                [](TFPubSubSubscription* subscription, TF_Status* out_status) noexcept
            {
                auto res = TFPubSubSubscriptionOps::from_handle(subscription).ack();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .nack =
                [](TFPubSubSubscription* subscription, TF_Status* out_status) noexcept
            {
                auto res = TFPubSubSubscriptionOps::from_handle(subscription).nack();
                if (!res) {
                    res.error().to_c(out_status);
                }
            },
            .seek =
                [](TFPubSubSubscription* subscription,
                   const TF_String* position,
                   TF_Status* out_status) noexcept
            {
                auto res = TFPubSubSubscriptionOps::from_handle(subscription)
                               .seek(ice::sonic::String::wrap(position));
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
                auto res = TFPubSubSubscriptionOps::from_handle(subscription)
                               .get_lag(completion, user_data);
                if (!res) {
                    res.error().to_c(out_status);
                }
            },

        };
    }

    const ::TFPubSubSubscriptionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFPubSubSubscription& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFPubSubSubscriptionOps m_vtable;
    TFPubSubSubscription m_handle;
};

} // namespace ice::builder
