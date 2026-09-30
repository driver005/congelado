// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/pubsub/pubsub.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/pubsub/pubsub.h"

export module cc_ice_extern_pubsub_sonic:pubsub;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TF_PubSubOps : public ice::sonic::Runtime<TF_PubSubOps, TF_PubSubOps>
{
public:
    explicit TF_PubSubOps(TF_PubSubOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "pubsub";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void get_name(const ice::sonic::String& out_name) noexcept
    {
        m_ops->get_name(get_handle(), out_name.get_handle());
    }
};

} // namespace ice::sonic
