// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/watch.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/watch.h"

export module cc_ice_builder_store:watch;

import std;

export namespace ice::builder {

class TFStoreWatchOps
{
public:
    static TFStoreWatchOps* create(void* ctx) noexcept
    {
        return static_cast<TFStoreWatchOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreWatchOps* create(HandleT* handle) noexcept
    {
        return static_cast<TFStoreWatchOps*>(handle->plugin_data);
    }

    virtual ~TFStoreWatchOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> cancel() noexcept = 0;

    static TFStoreWatchOps* get_generic_vtable()
    {
        static TFStoreWatchOps vtable = {
            .struct_size = TF_TOREWATCH_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                delete TFStoreWatchOps::create(plugin_context);
            },
            .cancel =
                [](TFStoreWatch* watch) noexcept
            {
                auto* self = TFStoreWatchOps::create(watch);
                auto res = self->cancel();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };

        return &vtable;
    }

    builder::String get_name() const noexcept
    {
        builder::String result;
        m_ops->get_name(get_handle(), result.get_handle());
        return result;
    }
};

} // namespace ice::builder
