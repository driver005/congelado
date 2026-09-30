// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/watch.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/watch.h"

export module cc_ice_extern_store_builder:watch;

import std;

export namespace ice::builder {

class TFStoreWatchOps
{
public:
    TFStoreWatchOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TFStoreWatchOps(const TFStoreWatchOps&) = delete;
    TFStoreWatchOps& operator=(const TFStoreWatchOps&) = delete;

    static TFStoreWatchOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TFStoreWatchOps*>(ctx);
    }

    template<typename HandleT>
    static TFStoreWatchOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TFStoreWatchOps*>(handle->plugin_data);
    }

    virtual ~TFStoreWatchOps() = default;
    [[nodiscard]] virtual std::expected<void, ice::Status> cancel() noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TFStoreWatchOps{
            .struct_size = TF_TOREWATCH_STRUCT_SIZE,

            .destroy =
                [](void* plugin_context) noexcept
            {
                std::unique_ptr<TFStoreWatchOps>{&TFStoreWatchOps::from_handle(plugin_context)};
            },
            .cancel =
                [](TFStoreWatch* watch) noexcept
            {
                auto res = TFStoreWatchOps::from_handle(watch).cancel();
                if (!res) {
                    res.error().to_c(status);
                }
            },

        };
    }

    const ::TFStoreWatchOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TFStoreWatch& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TFStoreWatchOps m_vtable;
    TFStoreWatch m_handle;
};

} // namespace ice::builder
