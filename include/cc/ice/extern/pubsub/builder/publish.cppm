// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/publish.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/publish.h"
#include "include/c/extern/registration/registration.h"
#include "include/c/intern/status.h"
#include "include/c/intern/tstring.h"
#include "include/c/intern/vector.h"

export module cc_ice_extern_pubsub_builder:publish;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class TFPubSubPublishOps
{
public:
    explicit TFPubSubPublishOps(
        const ::TF_StatusOps* Status_ops,
        const ::TF_StringOps* String_ops,
        const ::TF_VectorOps* TF_VectorOps_ops
    ) noexcept :
        m_handle{.plugin_data = this}
    {
        m_Status_ops = Status_ops;
        m_String_ops = String_ops;
        m_TF_VectorOps_ops = TF_VectorOps_ops;
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
    virtual void publish(
        const ice::sonic::String& channel,
        const ice::sonic::String& payload,
        int retain,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void publish_batch(
        const ice::sonic::String& channel,
        const ice::sonic::TF_VectorOps& payloads,
        TFPubSubAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void flush(
        TFPubSubAckFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;
    virtual void get_retained(
        const ice::sonic::String& channel,
        TFPubSubRetainedFn completion,
        void* user_data,
        const ice::sonic::Status& out_status
    ) noexcept = 0;

    void get_generic_vtable(void (*create)(::TFPubSubPublish*)) noexcept
    {
        m_vtable = ::TFPubSubPublishOps{
            .struct_size = TF_OFFSET_OF_END(::TFPubSubPublishOps, get_retained),

            .create = create,
            .destroy =
                [](TFPubSubPublish* handle) noexcept
            {
                auto& self = TFPubSubPublishOps::from_handle(handle);
                self.destroy();
            },
            .publish =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   const TF_String* payload,
                   int retain,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubPublishOps::from_handle(publish);
                self.publish(
                    self.wrap(std::type_identity<ice::sonic::String>{}, channel),
                    self.wrap(std::type_identity<ice::sonic::String>{}, payload),
                    retain,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .publish_batch =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   const TF_Vector* payloads,
                   TFPubSubAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubPublishOps::from_handle(publish);
                self.publish_batch(
                    self.wrap(std::type_identity<ice::sonic::String>{}, channel),
                    self.wrap(std::type_identity<ice::sonic::TF_VectorOps>{}, payloads),
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .flush =
                [](TFPubSubPublish* publish,
                   TFPubSubAckFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubPublishOps::from_handle(publish);
                self.flush(
                    completion,
                    user_data,
                    self.wrap(std::type_identity<ice::sonic::Status>{}, out_status)
                );
            },
            .get_retained =
                [](TFPubSubPublish* publish,
                   const TF_String* channel,
                   TFPubSubRetainedFn completion,
                   void* user_data,
                   TF_Status* out_status) noexcept
            {
                auto& self = TFPubSubPublishOps::from_handle(publish);
                self.get_retained(
                    self.wrap(std::type_identity<ice::sonic::String>{}, channel),
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

    ice::sonic::TF_VectorOps
    wrap(std::type_identity<ice::sonic::TF_VectorOps>, const ::TF_Vector* handle) const noexcept
    {
        return ice::sonic::TF_VectorOps{m_TF_VectorOps_ops, const_cast<::TF_Vector*>(handle)};
    }

    const ::TFPubSubPublishOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFPubSubPublish& get_handle() const noexcept
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
            const_cast<::TFPubSubPublishOps*>(&m_vtable)
        );
    }

private:
    ::TFPubSubPublishOps m_vtable;
    ::TFPubSubPublish m_handle;

    const ::TF_StatusOps* m_Status_ops{nullptr};

    const ::TF_StringOps* m_String_ops{nullptr};

    const ::TF_VectorOps* m_TF_VectorOps_ops{nullptr};
};

} // namespace ice::builder
