// GENERATED FILE — DO NOT EDIT BY HAND.
// Produced by cc_abi_gen from include/c/extern/store/store.h. Re-run
// `bazel run //include/cc/abi_gen:cc_abi_gen -- generate --pilot` (or `make gen-cc-abi`) to
// regenerate; edits made directly to this file will be overwritten.

module;

#include "include/c/extern/store/store.h"

export module cc_ice_extern_store_builder:store;

import std;

export namespace ice::builder {

class TF_StoreOps
{
public:
    TF_StoreOps() noexcept :
        m_handle{.plugin_data = this}
    {
    }

    TF_StoreOps(const TF_StoreOps&) = delete;
    TF_StoreOps& operator=(const TF_StoreOps&) = delete;

    static TF_StoreOps& from_handle(void* ctx) noexcept
    {
        return *static_cast<TF_StoreOps*>(ctx);
    }

    template<typename HandleT>
    static TF_StoreOps& from_handle(HandleT* handle) noexcept
    {
        return *static_cast<TF_StoreOps*>(handle->plugin_data);
    }

    virtual ~TF_StoreOps() = default;
    virtual void destroy() noexcept = 0;
    virtual void get_name(const ice::sonic::String& out_name) noexcept = 0;

    void get_generic_vtable() noexcept
    {
        m_vtable = ::TF_StoreOps{
            .struct_size = TF_STORE_STRUCT_SIZE,
            .destroy =
                [](TF_Store* store) noexcept
            {
                TF_StoreOps::from_handle(store).destroy();
            },
            .get_name =
                [](TF_Store* store, TF_String* out_name) noexcept
            {
                TF_StoreOps::from_handle(store).get_name(ice::sonic::String::wrap(out_name));
            },

        };
    }

    const ::TF_StoreOps& get_vtable() const noexcept
    {
        return m_vtable;
    }

    const TF_Store& get_handle() const noexcept
    {
        return m_handle;
    }


private:
    ::TF_StoreOps m_vtable;
    TF_Store m_handle;
};

} // namespace ice::builder
