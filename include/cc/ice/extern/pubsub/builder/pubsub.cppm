// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/pubsub.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/pubsub.h"

export module cc_ice_extern_pubsub_builder:pubsub;

import std;

export namespace ice::builder {

class TF_PubSubOps
{
public:
    TF_PubSubOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_PubSubOps(const TF_PubSubOps&) = delete;
    TF_PubSubOps& operator=(const TF_PubSubOps&) = delete;

    static TF_PubSubOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_PubSubOps*>(ctx);
    }

    template<typename HandleT>
    static TF_PubSubOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_PubSubOps*>(handle->plugin_data);
    }

    virtual ~TF_PubSubOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_PubSubOps{
            .struct_size = TF_PUBSUB_STRUCT_SIZE,
            .destroy =
                [](TF_PubSub* pubsub) noexcept
            {
                TF_PubSubOps::from_handle(pubsub).destroy();
            },
            .get_name =
                [](TF_PubSub* pubsub, TF_String* out_name) noexcept
            {
                TF_PubSubOps::from_handle(pubsub).get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_PubSubOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_PubSub& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_PubSubOps m_vtable;
    TF_PubSub m_handle;
};

} // namespace ice::builder
