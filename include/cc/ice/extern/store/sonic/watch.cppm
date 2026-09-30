// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/watch.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/watch.h"

export module cc_ice_extern_store_sonic:watch;

import std;
import cc_abi_sonic_registration;

export namespace ice::sonic {

class TFStoreWatchOps : public ice::sonic::Runtime<TFStoreWatchOps, TFStoreWatchOps>
{
public:
    explicit TFStoreWatchOps(TFStoreWatchOps* ops, void* plugin_context) noexcept :
        Runtime(ops, plugin_context)
    {
    }

    static constexpr std::string_view domain_name = "store";

    void destroy() noexcept
    {
        m_ops->destroy(get_handle());
    }

    void cancel() noexcept
    {
        m_ops->cancel(get_handle());
    }
};

} // namespace ice::sonic
