// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/pubsub.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/pubsub.h"
#include "include/c/intern/registration.h"
#include "include/c/intern/tstring.h"

export module cc_ice_extern_pubsub_builder:pubsub;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class PubSub
{
public:
    explicit PubSub(const ::TF_StringOps* String_ops) noexcept :
        m_handle{.plugin_data = this}
    {
        m_String_ops = String_ops;
    }

    PubSub(const PubSub&) = delete;
    PubSub& operator=(const PubSub&) = delete;

    static PubSub& from_handle(void* ctx) noexcept
    {
        return *static_cast<PubSub*>(ctx);
    }

    template<typename HandleT>
    static PubSub& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<PubSub*>(handle->plugin_data);
    }

    virtual ~PubSub() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable(void (*create)(::TF_PubSub*)) noexcept
    {
        m_vtable = ::TF_PubSubOps{
            .struct_size = TF_OFFSET_OF_END(::TF_PubSubOps, get_name),

            .create = create,
            .destroy =
                [](::TF_PubSub* handle) noexcept
            {
                auto& self = PubSub::from_handle(handle);
                self.destroy();
            },
            .get_name =
                [](::TF_PubSub* pubsub, ::TF_String* out_name) noexcept
            {
                auto& self = PubSub::from_handle(pubsub);
                self.get_name(self.wrap(std::type_identity<ice::sonic::String>{}, out_name));
            },

        };
    }

    ice::sonic::String
    wrap(std::type_identity<ice::sonic::String>, const ::TF_String* handle) const noexcept
    {
        return ice::sonic::String{m_String_ops, handle};
    }

    const ::TF_PubSubOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TF_PubSub& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TF_PubSubOps*>(&m_vtable));
    }

private:
    ::TF_PubSubOps m_vtable;
    ::TF_PubSub m_handle;

    const ::TF_StringOps* m_String_ops{nullptr};
};

} // namespace ice::builder
