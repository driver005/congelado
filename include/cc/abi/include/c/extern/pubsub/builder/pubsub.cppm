// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/pubsub.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/pubsub.h"

export module cc_abi_builder_pubsub;

import std;
import cc_abi_primitives;
import cc_abi_sonic_intern;

export namespace ice::builder {

class TF_PubSubOps
{
public:
    static TF_PubSubOps* create(void* ctx) noexcept
    {
        return static_cast<TF_PubSubOps*>(ctx);
    }

    template<typename HandleT>
    static TF_PubSubOps* create(HandleT* handle) noexcept
    {
        return static_cast<TF_PubSubOps*>(handle->plugin_data);
    }

    virtual ~TF_PubSubOps() = default;
    virtual ice::String get_name() const noexcept = 0;

    static TF_PubSubOps* get_generic_vtable()
    {
        static TF_PubSubOps vtable = {
            .struct_size = TF_PUBSUB_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TF_PubSubOps::create(plugin_context);
            },

            .get_name =
                [](void* plugin_context, TF_String* out) noexcept
            {
                auto* self = TF_PubSubOps::create(plugin_context);
                auto result = self->get_name();
                result.to_c(out);
            },

        };

        return &vtable;
    }
};

} // namespace ice::builder
