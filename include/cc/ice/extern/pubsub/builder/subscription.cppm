// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/subscription.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/subscription.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_pubsub_builder:subscription;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFPubSubSubscriptionOps
{
public:
    explicit TFPubSubSubscriptionOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
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
    virtual void destroy() noexcept = 0;
    virtual void unsubscribe() noexcept = 0;
    virtual void ack(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void nack(const ice::sonic::Status& out_status) noexcept = 0;
    virtual void
    seek(const ice::sonic::String& position, const ice::sonic::Status& out_status) noexcept = 0;
    virtual void get_lag(
        TFPubSubIntFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFPubSubSubscription*)) noexcept
    {
        m_vtable = ::TFPubSubSubscriptionOps{
            .struct_size = TF_OFFSET_OF_END(::TFPubSubSubscriptionOps, get_lag),

            .create = create,
            .destroy =
                [](TFPubSubSubscription* handle) noexcept
            {
                auto& self = TFPubSubSubscriptionOps::from_handle(handle);
                self.destroy();
            },
            .unsubscribe =
                [](TFPubSubSubscription* subscription) noexcept
            {
                auto& self = TFPubSubSubscriptionOps::from_handle(subscription);
                self.unsubscribe();
            },
            .ack =
                [](TFPubSubSubscription* subscription, TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubSubscriptionOps::from_handle(subscription);
                self.ack(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .nack =
                [](TFPubSubSubscription* subscription, TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubSubscriptionOps::from_handle(subscription);
                self.nack(self.wrap(std::type_identity<ice::sonic::Status>{}, out_status));
            },
            .seek =
                [](TFPubSubSubscription* subscription,
                   const TF_String* position,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubSubscriptionOps::from_handle(subscription);
                self.seek(
                    self.wrap(std::type_identity<ice::sonic::String>{}, position),
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_lag =
                [](TFPubSubSubscription* subscription,
                   TFPubSubIntFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubSubscriptionOps::from_handle(subscription);
                self.get_lag(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },

        };
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

    const ::TFPubSubSubscriptionOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFPubSubSubscription& get_handle() const noexcept
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
            const_cast<::TFPubSubSubscriptionOps*>(&m_vtable)
        );
    }

private:
    ::TFPubSubSubscriptionOps m_vtable;
    ::TFPubSubSubscription m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
