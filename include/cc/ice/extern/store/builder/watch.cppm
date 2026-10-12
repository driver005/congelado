// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/watch.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/watch.h"
#include "include/c/intern/registration.h"

export module cc_ice_extern_store_builder:watch;

import std;
import cc_ice_intern_sonic;

export namespace ice::builder {

class StoreWatch
{
public:
    explicit StoreWatch() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    StoreWatch(const StoreWatch&) = delete;
    StoreWatch& operator=(const StoreWatch&) = delete;

    static StoreWatch& from_handle(void* ctx) noexcept
    {
        return *static_cast<StoreWatch*>(ctx);
    }

    template<typename HandleT>
    static StoreWatch& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<StoreWatch*>(handle->plugin_data);
    }

    virtual ~StoreWatch() = default;
    virtual void destroy() noexcept = 0;
    virtual void cancel() noexcept = 0;

    void get_generic_vtable(void (*create)(::TFStoreWatch*)) noexcept
    {
        m_vtable = ::TFStoreWatchOps{
            .struct_size = TF_OFFSET_OF_END(::TFStoreWatchOps, cancel),

            .create = create,
            .destroy =
                [](::TFStoreWatch* handle) noexcept
            {
                auto& self = StoreWatch::from_handle(handle);
                self.destroy();
            },
            .cancel =
                [](::TFStoreWatch* watch) noexcept
            {
                auto& self = StoreWatch::from_handle(watch);
                self.cancel();
            },

        };
    }

    const ::TFStoreWatchOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const ::TFStoreWatch& get_handle() const noexcept
    {
        return m_handle;
    }

    void register_ops(
        const ice::sonic::Registration& registry,
        const ice::sonic::String& type,
        const ice::sonic::String& provider
    ) const noexcept
    {
        registry.register_op(type, provider, const_cast<::TFStoreWatchOps*>(&m_vtable));
    }

private:
    ::TFStoreWatchOps m_vtable;
    ::TFStoreWatch m_handle;
};

} // namespace ice::builder
